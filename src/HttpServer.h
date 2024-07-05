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

#ifndef MAU_HTTPSERVER__H
#define MAU_HTTPSERVER__H

#ifndef  MAU_GLOBAL__H
   #include "Global.h"
#endif

#pragma push_macro("DELETE")
#undef DELETE

#pragma push_macro("new")
#undef new
#include <QtCore/QString>
#include <QtCore/QByteArray>
#include <QtCore/QHash>
#include <QtCore/QVariantMap>
#pragma pop_macro("new")

//****************************************************************************
//!
//! \brief Abstract base class for a HTTP server implementation.
//!
//****************************************************************************

namespace mau {

class MAUCPPHTTPSERVER_EXPORT HttpServer
{
public:
   virtual ~HttpServer() {}

   enum ProtocolVersion {
      HTTP_1_1
      //HTTP_2  // planned feature
   };

   enum HttpMethod {
      UNKNOWN  = 0x0000,
      GET      = 0x0001,
      POST     = 0x0002,
      PUT      = 0x0004,
      DELETE   = 0x0008,
      HEAD     = 0x0010,
      OPTIONS  = 0x0020,
      PATCH    = 0x0040,

      ALL = GET | PUT | DELETE | POST | HEAD | OPTIONS | PATCH
   };

   enum SslKeyAlgorithm {
      RSA,
      DSA,
      EllipticCurve,
      DiffieHellman
   };

   enum SslEncoding {
      PEM,
      DER
   };

   struct HttpRequest {
      ProtocolVersion protocolVersion = HTTP_1_1;  //!< Protcol version
      HttpMethod method;                           //!< Request method
      QHash<QString, QString> headers;             //!< The headers of the request
      QByteArray body;                             //!< Request body
   };

   struct HttpResponse {
      ProtocolVersion protocolVersion = HTTP_1_1;  //!< Protcol version
      int statusCode;                              //!< Response status code
      QHash<QString, QString> headers;             //!< The headers of the response
      QByteArray body;                             //!< Response body
   };

   struct PathInfo {
      QString path;                                //!< The URL path
      QHash<QString, QString> variables;           //!< Names and values of path variables
      QString multiLevel;                          //!< Path that matched the multi level '#' wildcard
      QHash<QString, QString> query;               //!< The query component of the URI
   };

   QString Address();
      //!< \brief Retrieves the address for this server.
      //!< This is the IP address the server will be reachable under.
      //!< \return Server address.
      //!< \sa HttpServer::Address(QString) to set the address.

   void Address(const QString& address);
      //!< \brief Sets the address for this server.
      //!< The server address is the IP address under which it is reachable.
      //!< \param address The IP address.
      //!< \sa HttpServer::Address() to get the address.

   int Port();
      //!< \brief Retrieve the port number of this server.
      //!< The port under which the server is reachable.
      //!< \return The port number.
      //!< \sa HttpServer::Port(int) to set the port.

   void Port(int port);
      //!< \brief Sets the port number of this server.
      //!< Sets the the port under which the server will be reachable.
      //!< \param port The port number.
      //!< \sa HttpServer::Port() to get the port.

   bool IsHttps();
      //!< \brief If this server is a HTTPS server.
      //!< If this server is configured with a server certificate and a private
      //!< key, than it will use HTTPS instead of HTTP.
      //!< \return If this server is configured to use HTTPS.

   bool Start();
      //!< \brief Starts the server.
      //!< \return If the server was started.
      //!< \sa HttpServer::Stop() to stop the server.

   bool Stop();
      //!< \brief Stops the server.
      //!< \return If the server was stopped.
      //!< \sa HttpServer::Start() to start the server.

   bool Running();
      //!< \brief Check whether the server is running or not.
      //!< \return True if the server is running, false if not.

   bool AddEndpoint(const QString& endpoint, HttpMethod method = ALL);
      //!< \brief Adds an endpoint to the server.
      //!< When a request with the given endpoint and method is received on the server,
      //!< HttpServer::OnRequest(QString, QString, QHash<QString, QString>, QString)
      //!< will be called.
      //!< \param endpoint Endpoint which should be handled by this server.
      //!< \param method   HTTP request method that should be routed.
      //!< \return bool    If the endpoint was added.
      //!< \sa HttpServer::OnRequest(QString, QString, QHash<QString, QString>, QString)
      //!<     for the callback.
      //!< \sa HttpServer::RemoveEndpoint(QString, HttpMethod)

   bool RemoveEndpoint(const QString& endpoint, HttpMethod method);
      //!< \brief Removes an endpoint from the server.
      //!< \param endpoint Endpoint which should be removed from this server.
      //!< \param method   HTTP request method whose route should be removed.
      //!< \return bool    If the endpoint was removed.
      //!< \sa HttpServer::AddEndpoint(QString, HttpMethod)

   bool SetCertificate(const QByteArray& certificateData, SslEncoding encoding);
      //!< \brief Sets the server certificate.
      //!< For SSL/TLS encrypted connections a server SSL certificate and
      //!< private key have to be set. To set the private key, use
      //!< HttpServer::SetPrivateKey(QByteArray, SslKeyAlgorithm, SslEncoding).
      //!< \param certificateData   The data of the server certificate.
      //!< \param encoding          SSL certificate encoding to use.
      //!< \return Whether the certificate could be set or not.
      //!< \sa HttpServer::SetPrivateKey(QByteArray, SslEncoding, SslKeyAlgorithm, QString)
      //!      for setting the private key.

   bool SetPrivateKey(const QByteArray& keyData, SslEncoding encoding, SslKeyAlgorithm algorithm, const QString& passphrase);
      //!< \brief Sets the private key of the server certificate.
      //!< For SSL/TLS encrypted connections a server SSL certificate and
      //!< private key have to be set. To set the certificate, use
      //!< SetCertificate::SetPrivateKey(QByteArray, SslEncoding).
      //!< \param keyData     The data of the private key.
      //!< \param encoding    SSL certificate encoding to use.
      //!< \param algorithm   The key algorithm of the private key.
      //!< \param passphrase  Passphrase for the private key.
      //!< \return Whether the private key could be set or not.
      //!< \sa HttpServer::SetCertificate(QByteArray, SslEncoding) for setting
      //!<     the certificate.

protected:
   virtual QString AddressImpl() = 0;
   virtual void AddressImpl(const QString& address) = 0;
   virtual int PortImpl() = 0;
   virtual void PortImpl(int port) = 0;

   virtual bool IsHttpsImpl() = 0;

   virtual bool StartImpl() = 0;
   virtual bool StopImpl() = 0;

   virtual bool AddEndpointImpl(const QString& endpoint, HttpMethod method) = 0;
   virtual bool RemoveEndpointImpl(const QString& endpoint, HttpMethod method) = 0;

   virtual bool SetCertificateImpl(const QByteArray& certificateData, SslEncoding encoding) = 0;
   virtual bool SetPrivateKeyImpl(const QByteArray& keyData, SslEncoding encoding, SslKeyAlgorithm algorithm, const QString& passphrase) = 0;

   virtual HttpResponse OnRequest(const QString& endpoint, const QString& url, const PathInfo& pathInfo, const HttpRequest& request) { return HttpResponse();  };
      //!< \brief Called when a new request was received.
      //!< #endpoint contains the registered endpoint (with placeholders) and path
      //!< the actual URL path.
      //!< \param endpoint Endpoint that was registered.
      //!< \param url      URL that was called.
      //!< \param pathInfo Information about the path and path variables.
      //!< \param request  Request data send by the client.
      //!< \returns        The HTTPResponse containing all data for the server response.

private:
   bool started = false;

};

}

#pragma pop_macro("DELETE")
#endif