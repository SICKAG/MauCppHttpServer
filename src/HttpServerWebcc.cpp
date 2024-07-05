//*****************************************************************************
//
// Copyright (C) 2024 SICK AG
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, write to SICK AG, Erwin-Sick-Str. 1,
// 79183 Waldkirch.
//!
//*****************************************************************************

#include "Global.h"
#include "Exception.h"

#include "HttpServerWebcc.h"

#pragma push_macro("new")
#undef new
#include <QtCore/QStringList>
#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QUrl>
#include <QtCore/QPair>
#include <QtCore/QVariant>
#include <QtCore/QRegularExpression>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QSslCertificate>
#include <QtNetwork/QSslKey>
#pragma pop_macro("new")

#include <boost/asio/ip/tcp.hpp>

#include "webcc/url.h"
#include "webcc/server.h"
#include "webcc/ssl_server.h"
#include "webcc/response_builder.h"

#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

#define Ex(id)           Exception("HttpServerWebcc::"#id"Ex", Exception::error, msg##id##Ex).LocHere()
#define Warn(id)         Exception("HttpServerWebcc::"#id"", Exception::warning, msg##id##Warn).LocHere()
#define ThrowUnknownEx() Exception("HttpServerWebcc::UnknownEx", Exception::error, msgUnknownEx).LocHere()

namespace mau {

//*****************************************************************************
//!
//! \brief Private implementation class for HttpServerWebcc.
//!
//*****************************************************************************

class HttpServerWebcc::HttpServerWebccPrivate
{
private:
   // Webcc View that handles all HTTP requests.
   class RootView : public webcc::View {
   public:
      RootView(HttpServerWebcc::HttpServerWebccPrivate* parent)
         : webcc::View(), parent(parent) {}
      webcc::ResponsePtr Handle(webcc::RequestPtr request) override;

   private :
      HttpServerWebcc::HttpServerWebccPrivate* parent;
   };

   // Thread that on which the server tuns.
   class ServerThread : public QThread {
   public:
       ServerThread(webcc::Server* server) : server(server) {}

   protected:
       virtual void run();

   private:
       webcc::Server* server;
   };

public:
   HttpServerWebccPrivate(HttpServerWebcc* parent);
   ~HttpServerWebccPrivate() {}

   bool Start(const QHostAddress& address, int& port);
   bool Stop();

   bool AddEndpoint(const QString& endpoint, HttpMethod method);
   bool RemoveEndpoint(const QString& endpoint, HttpMethod method);

   bool SetCertificate(const QByteArray& data, SslEncoding encoding);
   bool SetPrivateKey(const QByteArray& data, SslEncoding encoding, SslKeyAlgorithm algorithm, const QString& passphrase);

   bool IsHttps();

private:
   struct UrlMatch {
      bool match = false;
      int level = 0;  //!< The level of the match. The higher the number, the more path variables were used.
      QString endpoint;
      QHash<QString, QString> pathVariables;
      QString multiLevel;
   };

   int                GetFreePort(int port);
   UrlMatch           Matches(const QString& endpoint, const QString& url);
   webcc::ResponsePtr HandleRequest(webcc::RequestPtr requestData);
   webcc::ResponsePtr ProcessRequest(const UrlMatch& match, webcc::RequestPtr requestData);
   QString            MapMethod(HttpMethod method);
   HttpMethod         MapMethod(QString method);

private:
   mutable QMutex processing;

   HttpServerWebcc* parent;
   webcc::Server* server;
   std::unique_ptr<ServerThread> serverThread;
   QHash<QPair<QString, HttpMethod>, QString> endpoints;

   QString serverName;
   QList<QString> reservedHeaders;
   QRegularExpression pathVariableRx;
   QRegularExpression pathVariableExactRx;

   QSslCertificate certificate;
   QSslKey privateKey;

   static EventMsg msgUnknownEx;
   static EventMsg msgFailedToStartEx;
   static EventMsg msgInvalidEndpointEx;
   static EventMsg msgInvalidEndpointHashtagWildcardEx;
   static EventMsg msgInvalidCharacterInEndpointEx;
   static EventMsg msgUnsupportedHttpMethodEx;
   static EventMsg msgAmbiguousEndpointEx;
   static EventMsg msgInvalidStatusCodeEx;
   static EventMsg msgReserverHeaderEx;
   static EventMsg msgMissingCertificateEx;
   static EventMsg msgMissingPrivateKeyEx;
   static EventMsg msgHeadWithBodyWarn;
};

EventMsg HttpServerWebcc::HttpServerWebccPrivate::msgUnknownEx = EventMsg({
   { "en-US", "Unknown Exception occurred." },
   { "de-DE", "Unbekannte Exception aufgetreten." }
});

EventMsg HttpServerWebcc::HttpServerWebccPrivate::msgFailedToStartEx = EventMsg({
   { "en-US", "Couldn't start http server: \"%1\"." },
   { "de-DE", "Http-Server konnte nicht gestartet werden: \"%1\"." }
});

EventMsg HttpServerWebcc::HttpServerWebccPrivate::msgInvalidEndpointEx = EventMsg({
   { "en-US", "Invalid endpoint '%1'." },
   { "de-DE", "Ungültiger Endpunkt '%1'." }
});

EventMsg HttpServerWebcc::HttpServerWebccPrivate::msgInvalidEndpointHashtagWildcardEx = EventMsg({
   { "en-US", "Invalid endpoint '%1': '#' wildcard has to be at the end." },
   { "de-DE", "Ungültiger Endpunkt '%1': '#' Wildcard muss am Ende stehen." }
});

EventMsg HttpServerWebcc::HttpServerWebccPrivate::msgInvalidCharacterInEndpointEx = EventMsg({
   { "en-US", "Invalid character '%1' in the endpoint path. This is a reserved character for path variables." },
   { "de-DE", "Ungültiges Zeichen '%1' im Endpunkt-Pfad. Dies ist ein reserviertes Zeichen für Pfad-Variablen." }
  });

EventMsg HttpServerWebcc::HttpServerWebccPrivate::msgUnsupportedHttpMethodEx = EventMsg({
   { "en-US", "Unsupported HTTP request method." },
   { "de-DE", "Nicht unterstützte HTTP-Request Methode." }
 });

EventMsg HttpServerWebcc::HttpServerWebccPrivate::msgAmbiguousEndpointEx = EventMsg({
   { "en-US", "Ambigous endpoint '%1'. Registered endpoint '%2' already routes to this endpoint." },
   { "de-DE", "Mehrdeutiger Endpunkt '%1'. Registrierter Endpunkt '%2' routet bereits zu diesem Endpunkt." }
});

EventMsg HttpServerWebcc::HttpServerWebccPrivate::msgInvalidStatusCodeEx = EventMsg({
   { "en-US", "HTTP server '%1', Endpoint '%2': Invalid status code '%3'. The HTTP server returned an non-standardize status codes." },
   { "de-DE", "HTTP-Server '%1', Endpunkt '%2': Ungültiger Status-Code '%3'. Der HTTP-Server hat einen nicht standardisierte Status-Codes zurückgegeben." }
});

EventMsg HttpServerWebcc::HttpServerWebccPrivate::msgReserverHeaderEx = EventMsg({
   { "en-US", "HTTP server '%1', Endpoint '%2': The response header '%3' is set by the server automatically. Overwriting it is not allowed." },
   { "de-DE", "HTTP-Server '%1', Endpunkt '%2': Der Antwort-Header '%3' wird automatisch vom Server gesetzt. Ihn zu überschreiben ist nicht erlaubt." }
});

EventMsg HttpServerWebcc::HttpServerWebccPrivate::msgMissingCertificateEx = EventMsg({
   { "en-US", "HTTP server '%1' has a private key set but is missing a server SSL certificiate." },
   { "de-DE", "HTTP-Server '%1' hat einen privaten Schlüssel gesetzt aber es fehlt ein Server SSL-Zertifikat." }
});

EventMsg HttpServerWebcc::HttpServerWebccPrivate::msgMissingPrivateKeyEx = EventMsg({
   { "en-US", "HTTP server '%1' has a server SSL certificate set but is missing a private key." },
   { "de-DE", "HTTP-Server '%1' hat ein Server SSL-Zertifikat gesetzt aber es fehlt ein privater Schlüssel." }
 });

EventMsg HttpServerWebcc::HttpServerWebccPrivate::msgHeadWithBodyWarn = EventMsg({
   { "en-US", "HTTP server '%1', Endpoint '%2': The callback for HEAD requests returns a response body. HEAD requests may not have a response body and the returned body will be ignored." },
   { "de-DE", "HTTP-Server '%1', Endpunkt '%2': Die Callback-Funktion für HEAD-Anfragen gibt einen Antwort-Body zurück. HEAD-Anfrage dürfen keinen Antwort-Body haben und der zurückgegebene Body wird ignoriert." }
});

//*****************************************************************************
//! Handle implementation of the webcc::View that handles every HTTP request.
//*****************************************************************************
webcc::ResponsePtr  HttpServerWebcc::HttpServerWebccPrivate::RootView::Handle(webcc::RequestPtr request) {
   return parent->HandleRequest(request);
}

//*****************************************************************************
//! Server thread loop implementation.
//*****************************************************************************
void HttpServerWebcc::HttpServerWebccPrivate::ServerThread::run()
{
   server->Run();
}

//*****************************************************************************
//! Constructor
//*****************************************************************************
HttpServerWebcc::HttpServerWebccPrivate::HttpServerWebccPrivate(HttpServerWebcc* parent) :
   parent(parent),
   reservedHeaders({ "Server", "Content-Length", "Connection", "Date" }),
   pathVariableRx("\\{(.+)\\}", QRegularExpression::InvertedGreedinessOption)
{
   pathVariableExactRx = QRegularExpression(QRegularExpression::anchoredPattern(pathVariableRx.pattern()), QRegularExpression::InvertedGreedinessOption);
}

//*****************************************************************************
//!
//! \brief Starts the server.
//! \param address   The host address of the server.
//! \param port      Port to listen to. 0 if the port should be auto assigned.
//!                  Will be set to the actual port.
//! \returns bool    If the server was started.
//!
//*****************************************************************************
bool HttpServerWebcc::HttpServerWebccPrivate::Start(const QHostAddress& address, int& port)
{
   port = GetFreePort(port);
   QString serverAddress = address.toString() + ":" + QString::number(port);

   bool sslEnabled = false;
   if (!certificate.isNull() && privateKey.isNull()) {
      Ex(MissingPrivateKey).Arg("https://" + serverAddress).Raise();
   } else if (certificate.isNull() && !privateKey.isNull()) {
      Ex(MissingCertificate).Arg("https://" + serverAddress).Raise();
   } else if (!certificate.isNull() && !privateKey.isNull()) {
      // HTTPs server
      server = new webcc::SslServer(boost::asio::ip::tcp::v4(), port);

      // Setup ssl context
      QByteArray certificateData = certificate.toPem();
      QByteArray privateKeyData  = privateKey .toPem();

      auto& sslContext = static_cast<webcc::SslServer*>(server)->ssl_context();
      sslContext.set_options(boost::asio::ssl::context::default_workarounds);
      sslContext.use_certificate_chain(boost::asio::const_buffer(reinterpret_cast<const void*>(certificateData.constData()), certificateData.size()));
      sslContext.use_private_key      (boost::asio::const_buffer(reinterpret_cast<const void*>(privateKeyData.constData()) , privateKeyData.size()), boost::asio::ssl::context::pem);

      sslEnabled = true;
   } else {
      // HTTP server
      server = new webcc::Server(boost::asio::ip::tcp::v4(), port);
   }

   // Route every request to a RootView view.
   bool routed = server->Route(
      webcc::UrlRegex("/.*"),             // URL regex
      std::make_shared<RootView>(this),   // View
      {                                   // Methods
         webcc::methods::kGet,
         webcc::methods::kPost,
         webcc::methods::kPut,
         webcc::methods::kDelete,
         webcc::methods::kPatch,
         webcc::methods::kHead,
         webcc::methods::kOptions
      }
   );

   if (!routed)
      Ex(FailedToStart).Arg("Routing failed.").Raise();

   // Create and start server thread. Necessary because server->run() is blocking.
   serverThread = std::make_unique<ServerThread>(server);
   serverThread->start();

   QString protocol = sslEnabled ? "https://" : "http://";
   serverName = protocol + serverAddress;
   return true;
}

//*****************************************************************************
//!
//! \brief Stops the server.
//! \returns bool If the server was stopped.
//!
//*****************************************************************************
bool HttpServerWebcc::HttpServerWebccPrivate::Stop()
{
   server->Stop();
   serverThread->wait();
   delete server;

   return true;
}

//*****************************************************************************
//!
//! \brief Adds an endpoint to the server.
//! When a request to the server for this endpoint and the given method is
//! received, the OnRequest() method of the HttpServerWebcc will be called.
//! The endpoint can contain single level path variables ({<name>}) and one
//! multi level wildcard (#) at the end.
//! Checks if any endpoint is invalid or if an already registered endpoint
//! routes to #endpoint already.
//!
//! \param   endpoint   Endpoint to add.
//! \param   method     HTTP request method for the endpoint.
//! \returns bool       If the endpoint could be added or not.
//!
//*****************************************************************************
bool HttpServerWebcc::HttpServerWebccPrivate::AddEndpoint(const QString& endpoint, HttpServer::HttpMethod method)
{
   // Check if '#' is a the end of the endpoint
   if (endpoint.contains("#") && endpoint.indexOf("#") != endpoint.length() - 1)   // indexOf returns first occurence
      Ex(InvalidEndpointHashtagWildcard).Arg(endpoint).Raise();

   // Check if the endpoint is valid
   QString endpointAdjusted = endpoint;

   // Replace the actual name of the path variable to a generic one, so we can check if the endpoint is already routed to.
   endpointAdjusted.replace(pathVariableRx, "[variableName]");

   // Replace '#'
   endpointAdjusted = endpointAdjusted.replace("#", "hashtag");

   // Check if there are invalid characters in the URL path. For now only '{' and '}'.
   if (endpointAdjusted.contains("{"))
      Ex(InvalidCharacterInEndpoint).Arg("{").Raise();
   if (endpointAdjusted.contains("}"))
      Ex(InvalidCharacterInEndpoint).Arg("}").Raise();

   // Check if the endpoint is valid
   QUrl url("localhost");
   url.setPath(endpointAdjusted);
   if (!url.isValid())
      Ex(InvalidEndpoint).Arg(endpoint).Raise();

   //QHttpServerRequest::Method httpMethod = MapMethod(method);
   //if (httpMethod == QHttpServerRequest::Method::Unknown)
   //   Ex(UnsupportedHttpMethod).Raise();

   QPair<QString, HttpMethod> key(endpointAdjusted, method);
   if (endpoints.contains(key))
      Ex(AmbiguousEndpoint).Arg(endpoint).Arg(endpoints[key]).Raise();

   endpoints.insert(key, endpoint);
   return true;
}

//*****************************************************************************
//!
//! \brief Removes an endpoint from the server.
//! The endpoint can no longer be reached after it was removed.
//!
//! \param   endpoint   Endpoint to remove.
//! \param   method     HTTP request method for the endpoint.
//! \returns bool       If the endpoint could be removed or not.
//!
//*****************************************************************************
bool HttpServerWebcc::HttpServerWebccPrivate::RemoveEndpoint(const QString& endpoint, HttpServer::HttpMethod method)
{
   bool endpointFound = false;

   for (auto it = endpoints.constBegin(); it != endpoints.constEnd(); it++) {
      if (it.value() == endpoint && it.key().second == method) {
         endpoints.remove(it.key());
         endpointFound = true;
         break;
      }
   }

   return endpointFound;
}

//*****************************************************************************
//!
//! \brief Sets the server certificate.
//!
//! \param   data Server certificate.
//! \returns bool If the certificate was set or not.
//!
//*****************************************************************************
bool HttpServerWebcc::HttpServerWebccPrivate::SetCertificate(const QByteArray& data, HttpServer::SslEncoding encoding)
{
   QSsl::EncodingFormat qEncoding;
   switch (encoding) {
      case SslEncoding::PEM: qEncoding = QSsl::Pem; break;
      case SslEncoding::DER: qEncoding = QSsl::Der; break;
      default: return false;
   }

   certificate = QSslCertificate(data, qEncoding);

   return !certificate.isNull();
}

//*****************************************************************************
//!
//! \brief Sets the private key of the server certificate.
//!
//! \param   data Private key.
//! \returns bool If the private key was set or not.
//!
//*****************************************************************************
bool HttpServerWebcc::HttpServerWebccPrivate::SetPrivateKey(const QByteArray& data, HttpServer::SslEncoding encoding, HttpServer::SslKeyAlgorithm algorithm, const QString& passphrase)
{
   QSsl::KeyAlgorithm qAlgorithm;
   switch (algorithm) {
      case SslKeyAlgorithm::RSA:             qAlgorithm = QSsl::Rsa;  break;
      case SslKeyAlgorithm::DSA:             qAlgorithm = QSsl::Dsa;  break;
      case SslKeyAlgorithm::EllipticCurve:   qAlgorithm = QSsl::Ec;   break;
      //case SslKeyAlgorithm::DiffieHellman:   qAlgorithm = QSsl::Dh;   break; // Available with Qt 5.14
      default: return false;
   }

   QSsl::EncodingFormat qEncoding;
   switch (encoding) {
      case SslEncoding::PEM: qEncoding = QSsl::Pem; break;
      case SslEncoding::DER: qEncoding = QSsl::Der; break;
      default: return false;
   }

   QByteArray passphraseData = QByteArray();
   if (!passphrase.isNull())  // Might not be neccessary, just in case.
      passphraseData = passphrase.toUtf8().data();

   privateKey = QSslKey(data, qAlgorithm, qEncoding, QSsl::PrivateKey, passphraseData);

   return !privateKey.isNull();
}

//*****************************************************************************
//!
//! \brief If this server is a HTTPS server.
//! Unless provided with a server certificate and a private key, the server
//! will use HTTP. If configured, it is considered a HTTPS server.
//!
//! \returns If this server is configured to be a HTTPS server.
//!
//*****************************************************************************
bool HttpServerWebcc::HttpServerWebccPrivate::IsHttps()
{
   return !certificate.isNull() || !privateKey.isNull();
}

//*****************************************************************************
//!
//! \brief Determines a free TCP port.
//!
//! \param   port    Preferred port. Set to 0 if a random port should be returned.
//! \returns int     A free port.
//!
//*****************************************************************************
int HttpServerWebcc::HttpServerWebccPrivate::GetFreePort(int port) {
   QTcpSocket socket;
   if (!socket.bind(port, QAbstractSocket::DontShareAddress))
      Ex(FailedToStart).Arg(socket.errorString()).Raise();

   quint16 assignedPort = socket.localPort();
   socket.close();

   return assignedPort;
}

//*****************************************************************************
//!
//! \brief Checks if the #endpoint matches the #url.
//! #endpoint could contain wildcards and path variables and has to be matched
//! against the url path.
//!
//! \param   endpoint   Endpoint to check.
//! \param   url        The URL that was called.
//! \returns UrlMatch   Struct containing information about the match.
//!
//*****************************************************************************
HttpServerWebcc::HttpServerWebccPrivate::UrlMatch HttpServerWebcc::HttpServerWebccPrivate::Matches(const QString& endpoint, const QString& url)
{
   UrlMatch match;
   match.endpoint = endpoint;

   // Escape the endpoint, because the URL will be escaped as well.
   QUrl endpointUrl;
   endpointUrl.setPath(endpoint);
   QString escapedEndpoint = endpointUrl.path();

   QStringList urlLevels = url.split("/");
   QStringList endpointLevels = escapedEndpoint.split("/");

   if (urlLevels.length() < endpointLevels.length()) {
      return match;
   } else if (urlLevels.length() > endpointLevels.length()) {
      if (!escapedEndpoint.endsWith("#"))
         return match;
   }

   for (int i = 0; i < endpointLevels.length(); i++) {
      QRegularExpressionMatch pathVariableMatch = pathVariableExactRx.match(endpointLevels[i]);
      if (pathVariableMatch.hasMatch()) {    // Path variable in endpoint string
         match.pathVariables.insert(pathVariableMatch.captured(1), urlLevels[i]);
         match.level++;
      } else if (endpointLevels[i] == "#") { // '#' should be at the end (checked in AddEndpoint()).
         match.multiLevel = "/" + urlLevels.mid(i).join("/");  // Add leading '/'
         match.level = urlLevels.length() - i + 1;             // Should be one higher than the level a path covered by '#' would have, if it had a path variable per level
      } else if (urlLevels[i] != endpointLevels[i]) {
         return match;
      }
   }

   match.match = true;
   return match;
}

//*****************************************************************************
//!
//! \brief Checks if there is a registered endpoint for the request.
//! If an endpoint was found that matches the request url path, then
//! OnRequest() is called for this endpoint.
//!
//! \param   request             The actual request.
//! \returns webcc::ResponsePtr  Server response.
//!
//*****************************************************************************
webcc::ResponsePtr HttpServerWebcc::HttpServerWebccPrivate::HandleRequest(webcc::RequestPtr requestData)
{
   QHash<HttpMethod, UrlMatch> matches;

   for (auto it = endpoints.constBegin(); it != endpoints.constEnd(); it++) {
      QString endpoint = it.value();

      UrlMatch match = Matches(endpoint, QString::fromStdString(requestData->url().path()));
      if (match.match) {
         HttpMethod method = it.key().second;
         if (matches.contains(method)) {
            if (matches[method].level > match.level)
               matches[method] = match;
            else if (matches[method].level == match.level)
               ThrowUnknownEx();       // This shouldn't happen, because same level should be barred by HttpServerWebcc::HttpServerWebccPrivate::AddEndpoint()
         } else {
            matches[method] = match;   // First match for this request method
         }
      }
   }

   // Match found?
   if (matches.count() > 0) {
      HttpMethod requestMethod = MapMethod(QString::fromStdString(requestData->method()));
      if (matches.contains(requestMethod))
         return ProcessRequest(matches[requestMethod], requestData);
      else if (matches.contains(HttpMethod::ALL))
         return ProcessRequest(matches[HttpMethod::ALL], requestData);
      else
         return webcc::ResponseBuilder{}.Code(405)();   // Method Not Allowed
   }

   return webcc::ResponseBuilder{}.NotFound()();
}

//*****************************************************************************
//!
//! \brief Process a request for an endpoint.
//! Transforms all data for the OnRequest() callback and returns a
//! QHttpServerResponse to be returned to the client.
//!
//! \param   match               Match info, including the matched endpoint.
//! \param   requestData         Actual request data.
//! \param   url                 URL that was called.
//! \returns QHttpServerResponse Server response to be returned to the client.
//!
//*****************************************************************************
webcc::ResponsePtr HttpServerWebcc::HttpServerWebccPrivate::ProcessRequest(const UrlMatch& match, webcc::RequestPtr requestData)
{
   // Determine query component parameters
   webcc::UrlQuery urlQuery = requestData->query();
   QHash<QString, QString> query;
   for (size_t i = 0; i < urlQuery.Size(); i++) {
      webcc::UrlQuery::Parameter parameter = urlQuery.Get(i);
      query.insert(QString::fromStdString(parameter.first), QString::fromStdString(parameter.second));
   }

   // Determine headers
   webcc::Headers requestHeaders = requestData->headers();
   QHash<QString, QString> headers;
   for (size_t i = 0; i < requestHeaders.size(); i++) {
      webcc::Header header = requestHeaders.Get(i);
      headers.insert(QString::fromStdString(header.first), QString::fromStdString(header.second));
   }

   PathInfo path{
      QString::fromStdString(requestData->url().path()),
      match.pathVariables,
      match.multiLevel,
      query
   };

   HttpRequest request;
   request.method = MapMethod(QString::fromStdString(requestData->method()));
   request.headers = headers;
   request.body = QByteArray::fromStdString(requestData->data()); // TODO: Do we always get a StringBody?

   QString url = serverName + QString::fromStdString(requestData->url().path());
   if (!requestData->url().query().empty()) {
      url += "?" + QString::fromStdString(requestData->url().query());
   }

   // Call callback
   HttpResponse httpResponse = parent->OnRequest(match.endpoint, url, path, request);

   // Verfiy status code, QHttpServer only allows a certain set of status codes.
   // If we want to allow all status codes, a solution might be to create a derived class of QHttpServerResponse, that
   // overrides the virutal write(QHttpServerResponder) function and writes the status line itself.
   int code = httpResponse.statusCode;
   if (!(/*(code >= 100 && code <= 102) ||  */  // 1xx status codes are not supported for now. We would need a solution to call the callback repeatedly otherwise the HttpClient will block indefinitely when Wait() is called.
         (code >= 200 && code <= 208) || code == 226 ||
         (code >= 300 && code <= 308 && code != 306) ||
         (code >= 400 && code <= 417) ||
         (code >= 421 && code <= 424) || code == 426 ||
         (code >= 428 && code <= 429) || code == 431 || code == 451 ||
         (code >= 500 && code <= 508) || code == 510 || code == 511
      )) {
      Ex(InvalidStatusCode).Arg(serverName).Arg(match.endpoint).Arg(code).Log();
      return webcc::ResponseBuilder{}.InternalServerError()();
   }

   // Head request should not return a response body.
   if (request.method & HttpServer::HEAD && httpResponse.body.size() > 0) {
      Warn(HeadWithBody).Arg(serverName).Arg(match.endpoint).Log();
   }


   std::string_view contentType("application/octet-stream"); // Default Content-Type, see RFC 2616 7.2.1
   if (httpResponse.body.size() == 0)
      contentType = "application/x-empty";
   if (httpResponse.headers.contains("Content-Type")) // If the header is explicitly set, overwrite any default value.
      contentType = httpResponse.headers["Content-Type"].toUtf8().constData();

   webcc::ResponsePtr response = webcc::ResponseBuilder{}
      .Code(httpResponse.statusCode)
      .MediaType(contentType)
      .Body(httpResponse.body.toStdString())
      .Utf8()
      ();

   // Set headers
   for (QHash<QString, QString>::iterator i = httpResponse.headers.begin(); i != httpResponse.headers.end(); i++) {
      QString header = i.key();
      if (reservedHeaders.contains(header)) {
         Ex(ReserverHeader).Arg(serverName).Arg(match.endpoint).Arg(header).Log();
         return webcc::ResponseBuilder{}.InternalServerError()();
      }

      response->SetHeader(header.toUtf8().data(), i.value().toUtf8().data());
   }

   return response;
}

//*****************************************************************************
//!
//! \brief Maps the HttpMethod to a QString.
//! \param   method  The method to map.
//! \returns QString QString corresponding to #method.
//!
//*****************************************************************************
QString HttpServerWebcc::HttpServerWebccPrivate::MapMethod(HttpServerWebcc::HttpMethod method)
{
   #pragma push_macro("DELETE")
   #undef DELETE

   QString mappedMethod = "Unknown";
   switch (method) {
      case HttpServerWebcc::HttpMethod::UNKNOWN: mappedMethod = "Unknown";                break;
      case HttpServerWebcc::HttpMethod::GET:     mappedMethod = webcc::methods::kGet;     break;
      case HttpServerWebcc::HttpMethod::POST:    mappedMethod = webcc::methods::kPost;    break;
      case HttpServerWebcc::HttpMethod::PUT:     mappedMethod = webcc::methods::kPut;     break;
      case HttpServerWebcc::HttpMethod::DELETE:  mappedMethod = webcc::methods::kDelete;  break;
      case HttpServerWebcc::HttpMethod::PATCH:   mappedMethod = webcc::methods::kPatch;   break;
      case HttpServerWebcc::HttpMethod::HEAD:    mappedMethod = webcc::methods::kHead;    break;
      case HttpServerWebcc::HttpMethod::OPTIONS: mappedMethod = webcc::methods::kOptions; break;
      case HttpServerWebcc::HttpMethod::ALL:     mappedMethod = "All";                    break;
   }
   return mappedMethod;

   #pragma pop_macro("DELETE")
}

//*****************************************************************************
//!
//! \brief Maps a QString to a HttpMethod.
//! \param   method     The method to map.
//! \returns HttpMethod HttpMethod corresponding to #method.
//!
//*****************************************************************************
HttpServerWebcc::HttpMethod HttpServerWebcc::HttpServerWebccPrivate::MapMethod(QString method)
{
   #pragma push_macro("DELETE")
   #undef DELETE

   HttpMethod mappedMethod = UNKNOWN;
   if      (method == "Unknown")                { mappedMethod = UNKNOWN; }
   else if (method == webcc::methods::kGet)     { mappedMethod = GET;     }
   else if (method == webcc::methods::kPost)    { mappedMethod = POST;    }
   else if (method == webcc::methods::kPut)     { mappedMethod = PUT;     }
   else if (method == webcc::methods::kDelete)  { mappedMethod = DELETE;  }
   else if (method == webcc::methods::kPatch)   { mappedMethod = PATCH;   }
   else if (method == webcc::methods::kHead)    { mappedMethod = HEAD;    }
   else if (method == webcc::methods::kOptions) { mappedMethod = OPTIONS; }
   else if (method == "All")                    { mappedMethod = ALL;     }
   return mappedMethod;

   #pragma pop_macro("DELETE")
}



//*****************************************************************************
//! \category HttpServerWebcc methods
//*****************************************************************************

EventMsg HttpServerWebcc::msgInvalidAddressEx = EventMsg({
   { "en-US", "The address '%1' is not a valid server address." },
   { "de-DE", "Die Adresse '%1' ist keine gültige Server-Adresse." }
});

EventMsg HttpServerWebcc::msgInvalidPortEx = EventMsg({
   { "en-US", "'%1' is not a valid port number. Port numbers have to between 0 and 65535." },
   { "de-DE", "'%1' ist keine gültige Portnummer. Der Wert muss zwischen 0 und 65535 liegen." }
});

HttpServerWebcc::HttpServerWebcc() :
   p(new HttpServerWebccPrivate(this))
{
}

HttpServerWebcc::~HttpServerWebcc()
{
}

QString HttpServerWebcc::AddressImpl()
{
   QMutexLocker lock(&members);
   return address.toString();
}

void HttpServerWebcc::AddressImpl(const QString& address)
{
   QMutexLocker lock(&members);

   QHostAddress hostAddress;
   bool valid = hostAddress.setAddress(address);
   if (!valid)
      Ex(InvalidAddress).Arg(address).Raise();

   HttpServerWebcc::address = hostAddress;
}

int HttpServerWebcc::PortImpl()
{
   QMutexLocker lock(&members);
   return port;
}

void HttpServerWebcc::PortImpl(int port)
{
   QMutexLocker lock(&members);
   if (port < 0 || port > 65535)
      Ex(InvalidPort).Arg(port).Raise();

   HttpServerWebcc::port = port;
}

bool HttpServerWebcc::IsHttpsImpl()
{
   return p->IsHttps();
}

bool HttpServerWebcc::StartImpl()
{
   QMutexLocker lock(&members);
   return p->Start(address, port);
}

bool HttpServerWebcc::StopImpl()
{
   QMutexLocker lock(&members);
   return p->Stop();
}

bool HttpServerWebcc::AddEndpointImpl(const QString& endpoint, HttpServer::HttpMethod method)
{
   QMutexLocker lock(&members);
   return p->AddEndpoint(endpoint, method);
}

bool HttpServerWebcc::RemoveEndpointImpl(const QString& endpoint, HttpServer::HttpMethod method)
{
   QMutexLocker lock(&members);
   return p->RemoveEndpoint(endpoint, method);
}

bool HttpServerWebcc::SetCertificateImpl(const QByteArray& certificateData, HttpServer::SslEncoding encoding)
{
   QMutexLocker lock(&members);
   return p->SetCertificate(certificateData, encoding);
}

bool HttpServerWebcc::SetPrivateKeyImpl(const QByteArray& keyData, HttpServer::SslEncoding encoding, HttpServer::SslKeyAlgorithm algorithm, const QString& passphrase)
{
   QMutexLocker lock(&members);
   return p->SetPrivateKey(keyData, encoding, algorithm, passphrase);
}
}