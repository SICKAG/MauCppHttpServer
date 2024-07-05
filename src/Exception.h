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

#ifndef MAU_EXCEPTION__H
#define MAU_EXCEPTION__H

#ifndef  MAU_GLOBAL__H
   #include "Global.h"
#endif

#pragma push_macro("new")
#undef new
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QHash>
#pragma pop_macro("new")

#define LocHere() Location(THIS_FILE, __LINE__)

namespace mau {

typedef QHash<QString, QString> EventMsg;

class MAUCPPHTTPSERVER_EXPORT Exception
{
public:
   enum Severities {
      warning,
      error
   };

   Exception(const QString& id, Severities severity, const EventMsg& msg);
   virtual ~Exception();

public:
   /*! Supports polymorphic throwing. Call this method to raise the exception
       instead of using the C++ throw statement. */
   virtual void Raise() { throw *this; }

public:
   virtual Exception& Id(const QString& id);
   virtual Exception& Severity(Severities severity);
   virtual Exception& Msg(const EventMsg& msg);
   virtual Exception& Arg(int arg);
   virtual Exception& Arg(const QString& arg);
   virtual Exception& Location(const QString& file, int line);
   virtual Exception& Log();

   virtual Exception* Duplicate();

   void ClearArgs();

public:
   QString Id() const { return id; };
   Severities Severity() const { return severity; };
   QString File() const { return file; }
   int Line() const { return line; }
   EventMsg Msg() const;

protected:
   QString id;
   Severities severity;
   EventMsg msg;

   QString file;
   int line;

   QStringList args;

};
}

#endif
