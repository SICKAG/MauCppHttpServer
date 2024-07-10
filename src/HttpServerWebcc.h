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

#ifndef MAU_HTTPSERVERWEBCC__H
#define MAU_HTTPSERVERWEBCC__H

/***  System Includes  *******************************************************/

#pragma push_macro("new")
#undef new
#include <QtCore/QString>
#include <QtCore/QByteArray>
#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtCore/QThreadStorage>
#include <QtNetwork/QHostAddress>
#pragma pop_macro("new")

/***  Global Component Includes  *********************************************/

#ifndef  MAU_GLOBAL__H
   #include "Global.h"
#endif

#ifndef      MAU_HTTPSERVER__H
   #include "HttpServer.h"
#endif

/***  Exception Includes  ****************************************************/

#ifndef      MAU_EXCEPTION__H
   #include "Exception.h"
#endif

/***  Defines  ***************************************************************/

/***  Externals  *************************************************************/

/***  Class Hierarchy  *******************************************************/

//****************************************************************************
//!
//! \brief HTTP server based on the Webcc.
//!
//! https://github.com/sprinfall/webcc
//!
//****************************************************************************

namespace mau {

class MAUCPPHTTPSERVER_EXPORT HttpServerWebcc : public HttpServer
{

public:
   HttpServerWebcc();
   virtual ~HttpServerWebcc();
   HttpServerWebcc(const HttpServerWebcc&) = delete;
   HttpServerWebcc& operator=(const HttpServerWebcc&) = delete;

protected:
   virtual void ProtocolImpl(ServerProtocol protocol);
   virtual QString AddressImpl();
   virtual void AddressImpl(const QString& address);
   virtual int PortImpl();
   virtual void PortImpl(int port);

   virtual bool IsHttpsImpl();

   virtual bool StartImpl();
   virtual bool StopImpl();

   virtual bool AddEndpointImpl(const QString& endpoint, HttpMethod method);
   virtual bool RemoveEndpointImpl(const QString& endpoint, HttpMethod method);

   virtual bool SetCertificateImpl(const QByteArray& certificateData, SslEncoding encoding);
   virtual bool SetPrivateKeyImpl(const QByteArray& keyData, SslEncoding encoding, SslKeyAlgorithm algorithm, const QString& passphrase);

protected:
   class HttpServerWebccPrivate;
   std::unique_ptr<HttpServerWebccPrivate> p;   //!< Pointer to implementation of HTTP server with Webcc.

   ServerProtocol protocol;
   QHostAddress address;
   int port;

private:
   mutable QMutex members;                      //!< Mutex for the member variables.

   static EventMsg msgInvalidAddressEx;
   static EventMsg msgInvalidPortEx;
};

}

#endif
