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

#include <QtCore/QString>

#include <algorithm>

#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

std::vector<EventWriter*> Exception::eventWriters;

Exception::Exception(const QString& id, const QString& component, Event::Severities severity, const LocString& msg):
   id(id), component(component), severity(severity), msg(msg),
   file(""), line(0),
   formatted(false)
{

}

Exception::~Exception()
{
}

void Exception::AddEventWriter(EventWriter& writer)
{
   if (eventWriters.end() == std::find(eventWriters.begin(), eventWriters.end(), &writer)) {
      eventWriters.push_back(&writer);
   }
}

void Exception::RemoveEventWriter(EventWriter& writer)
{
   auto it = std::find(eventWriters.begin(), eventWriters.end(), &writer);
   if (eventWriters.end() != it) {
      eventWriters.erase(it);
   }
}

std::vector<EventWriter*> Exception::EventWriters()
{
   return eventWriters;
}

Exception& Exception::Id(const QString& id)
{
   Exception::id = id;
   return *this;
}

Exception& Exception::Component(const QString& component)
{
   Exception::component = component;
   return *this;
}

Exception& Exception::Severity(Event::Severities severity)
{
   Exception::severity = severity;
   return *this;
}

Exception& Exception::Msg(const LocString& msg)
{
   Exception::msg = msg;
   FormatInvalidate();
   return *this;
}

Exception& Exception::Kind(const QString& kind)
{
   kindOf.append(kind);
   kindOf.removeDuplicates();
   return *this;
}

Exception& Exception::Kind(const QStringList& kindList)
{
   kindOf.append(kindList);
   kindOf.removeDuplicates();
   return *this;
}

//*****************************************************************************
//!
//! Adds an integer argument.
//! \param arg The value itself.
//! \param fieldWidth Field width; excluding #pbin and #phex prefixes.
//! \param base Basis for integer serialization.
//! \param fillChar Prepended to get a length of fieldWidth.
//! \returns A reference to this instance.
//!
//*****************************************************************************

Exception& Exception::Arg(int arg, int fieldWidth, Base base, char fillChar)
{
   int baseInt; QString prefix;
   IntegerBase(base, baseInt, prefix);
   args.push_back(QString("%1%2").arg(prefix).arg(arg, fieldWidth, baseInt, QChar(fillChar)));
   FormatInvalidate();
   return *this;
}

//*****************************************************************************
//!
//! Similar to Arg(int, int, Base, char).
//!
//*****************************************************************************

Exception& Exception::Arg(unsigned int arg, int fieldWidth, Base base, char fillChar)
{
   int baseInt; QString prefix;
   IntegerBase(base, baseInt, prefix);
   args.push_back(QString("%1%2").arg(prefix).arg(arg, fieldWidth, baseInt, QChar(fillChar)));
   FormatInvalidate();
   return *this;
}

//*****************************************************************************
//!
//! Similar to Arg(int, int, Base, char).
//!
//*****************************************************************************

Exception& Exception::Arg(long arg, int fieldWidth, Base base, char fillChar)
{
   int baseInt; QString prefix;
   IntegerBase(base, baseInt, prefix);
   args.push_back(QString("%1%2").arg(prefix).arg(arg, fieldWidth, baseInt, QChar(fillChar)));
   FormatInvalidate();
   return *this;
}

//*****************************************************************************
//!
//! Similar to Arg(int, int, Base, char).
//!
//*****************************************************************************

Exception& Exception::Arg(__int64 arg, int fieldWidth, Base base, char fillChar)
{
   int baseInt; QString prefix;
   IntegerBase(base, baseInt, prefix);
   args.push_back(QString("%1%2").arg(prefix).arg(arg, fieldWidth, baseInt, QChar(fillChar)));
   FormatInvalidate();
   return *this;
}

//*****************************************************************************
//!
//! Similar to Arg(int, int, Base, char).
//!
//*****************************************************************************

Exception& Exception::Arg(unsigned __int64 arg, int fieldWidth, Base base, char fillChar)
{
   int baseInt; QString prefix;
   IntegerBase(base, baseInt, prefix);
   args.push_back(QString("%1%2").arg(prefix).arg(arg, fieldWidth, baseInt, QChar(fillChar)));
   FormatInvalidate();
   return *this;
}

//*****************************************************************************
//!
//! Similar to Arg(int, int, Base, char).
//!
//*****************************************************************************

Exception& Exception::Arg(unsigned long arg, int fieldWidth, Base base, char fillChar)
{
   int baseInt; QString prefix;
   IntegerBase(base, baseInt, prefix);
   args.push_back(QString("%1%2").arg(prefix).arg(arg, fieldWidth, baseInt, QChar(fillChar)));
   FormatInvalidate();
   return *this;
}

Exception& Exception::Arg(double arg)
{
   args.push_back(QString::number(arg));
   FormatInvalidate();
   return *this;
}

Exception& Exception::Arg(const QString& arg)
{
   args.push_back(arg);
   FormatInvalidate();
   return *this;
}

Exception& Exception::Arg(const char* arg)
{
   if (arg == 0) {
      args.push_back("");
   } else {
      args.push_back(arg);
   }
   FormatInvalidate();
   return *this;
}

Exception& Exception::Location(const QString& file, int line)
{
   Exception::file = file;
   Exception::line = line;
   FormatInvalidate();
   return *this;
}

Exception& Exception::Dump(const Class& originator)
{
   OStrStream dump;
   originator.Dump(dump);
   dumps.push_back(dump.Str());
   return *this;
}

Exception& Exception::Dump(const QString& data)
{
   dumps.push_back(data);
   return *this;
}

Exception& Exception::PrependDump(const Class& originator)
{
   OStrStream dump;
   originator.Dump(dump);
   dumps.push_front(dump.Str());
   return *this;
}

Exception& Exception::PrependDump(const QString& data)
{
   dumps.push_front(data);
   return *this;
}

Exception& Exception::Log()
{
   if (!formatted) {
      Format();
   }
   Event ev(id, component, severity, info, dumps);
   if ((file.size() > 0) && (line > 0)) {
      ev.Location(file, line);
   }
   ev.KindOf(kindOf);
   for (std::vector<EventWriter*>::iterator it = eventWriters.begin(); it != eventWriters.end(); it++) {
      (*it)->Report(ev);
   }
   return *this;
}

Exception* Exception::Duplicate()
{
   return new Exception(*this);
}

void Exception::ClearArgs()
{
   args.clear();
   FormatInvalidate();
}

bool Exception::KindOf(const QString& kind) const
{
   return kindOf.contains(kind);
}

QStringList Exception::KindList() const
{
   return kindOf;
}

LocString Exception::Info() const
{
   Format();
   return info;
}

LocString Exception::FullInfo() const
{
   Format();
   return fullInfo;
}

void Exception::FormatInvalidate()
{
   info.Clear();
   fullInfo.Clear();
   formatted = false;
}

void Exception::Format() const
{
   if (!formatted) {
      LocString base = PreFormat(msg);
      info.Clear();
      for (LocString::ConstIterator lit = base.Begin(); lit != base.End(); lit++) {
         QString string = *lit;
         for (std::vector<QString>::const_iterator ait = args.begin(); ait != args.end(); ait++) {
            string = string.arg(*ait);
         }
         info.Set(string, lit.LanguageCode());
      }

      fullInfo.Clear();
      for (LocString::ConstIterator lit = base.Begin(); lit != base.End(); lit++) {
         QString string = *lit;
         for (std::vector<QString>::const_iterator ait = args.begin(); ait != args.end(); ait++) {
            string = string.arg(*ait);
         }
         if ((file.size() > 0) && (line > 0)) {
            string += QString("\nFile %1, Line %2.").arg(file).arg(line);
         }
         for (std::deque<QString>::const_iterator dit = dumps.begin(); dit != dumps.end(); dit++) {
            string = string + "\n" + *dit;
         }
         fullInfo.Set(string, lit.LanguageCode());
      }
      formatted = true;
   }
}

//*****************************************************************************
//!
//! Derives settings for QString from the passed Base value:
//! the base as an integer value (e.g., 2 for binary and 16 for hex) and
//! a string prefix ("", "0b" or "0x").
//!
//*****************************************************************************

void Exception::IntegerBase(Base base, int& baseInt, QString& prefix)
{
   switch (base) {
      case pbin: prefix = "0b";
      case bin: baseInt = 2; break;

      case phex: prefix = "0x";
      case hex: baseInt = 16; break;

      default: prefix = "";
               baseInt = 10;
   }
}
