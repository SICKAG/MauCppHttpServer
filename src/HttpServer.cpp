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
#include "HttpServer.h"

#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

namespace mau {

QString HttpServer::Address() {
   return AddressImpl();
}

void HttpServer::Address(const QString& address) {
   AddressImpl(address);
}

int HttpServer::Port() {
   return PortImpl();
}

void HttpServer::Port(int port) {
   PortImpl(port);
}

bool HttpServer::IsHttps() {
   return IsHttpsImpl();
}

bool HttpServer::Start() {
   return started ? false : started = StartImpl();
}

bool HttpServer::Stop() {
   return !started ? false : !(started = !StopImpl());
}

bool HttpServer::Running() {
   return started;
}

bool HttpServer::AddEndpoint(const QString& endpoint, HttpServer::HttpMethod method) {
   return AddEndpointImpl(endpoint, method);
}

bool HttpServer::RemoveEndpoint(const QString& endpoint, HttpServer::HttpMethod method) {
   return RemoveEndpointImpl(endpoint, method);
}

bool HttpServer::SetCertificate(const QByteArray& certificateData, HttpServer::SslEncoding encoding) {
   return started ? false : SetCertificateImpl(certificateData, encoding);
}

bool HttpServer::SetPrivateKey(const QByteArray& keyData, HttpServer::SslEncoding encoding, HttpServer::SslKeyAlgorithm algorithm, const QString& passphrase) {
   return started ? false : SetPrivateKeyImpl(keyData, encoding, algorithm, passphrase);
}

}