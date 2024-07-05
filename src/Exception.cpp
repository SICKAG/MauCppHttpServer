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

#include <algorithm>

#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

namespace mau {

Exception::Exception(const QString& id, Exception::Severities severity, const EventMsg& msg):
   id(id), severity(severity), msg(msg),
   file(""), line(0)
{
}

Exception::~Exception()
{
}

Exception& Exception::Id(const QString& id)
{
   Exception::id = id;
   return *this;
}

Exception& Exception::Severity(Exception::Severities severity)
{
   Exception::severity = severity;
   return *this;
}

Exception& Exception::Msg(const EventMsg& msg)
{
   Exception::msg = msg;
   return *this;
}

Exception& Exception::Arg(int arg)
{
   args.push_back(QString::number(arg));
   return *this;
}

Exception& Exception::Arg(const QString& arg)
{
   args.push_back(arg);
   return *this;
}

Exception& Exception::Location(const QString& file, int line)
{
   Exception::file = file;
   Exception::line = line;
   return *this;
}

Exception& Exception::Log()
{
   // TODO
   return *this;
}

Exception* Exception::Duplicate()
{
   return new Exception(*this);
}

void Exception::ClearArgs()
{
   args.clear();
}

EventMsg Exception::Msg() const
{
   EventMsg base = msg;
   for (auto i = msg.cbegin(), end = msg.cend(); i != end; ++i) {
      QString string = i.value();
      for (const auto& arg : args) {
         string = string.arg(arg);
      }
      base[i.key()] = string;
   }
   return base;
}

}
