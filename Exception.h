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

#ifndef EXCEPTION__H
#define EXCEPTION__H

#pragma push_macro("new")
#undef new
#include <QtCore/QStringList>
#pragma pop_macro("new")

#define LocHere() Location(THIS_FILE, __LINE__)

typedef QHash<QString, QString> EventMsg;

class HTTPSERVER_EXPORT Exception
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
   /*! Base for int-to-string conversions. */
   enum Base {
      bin,     //!< Regular binary base.
      dec,     //!< Regular decimal base.
      hex,     //!< Regular hexadecimal base.
      pbin,    //!< Binary base with prefix "0b".
      phex     //!< Binary base with prefix "0x".
   };

   virtual Exception& Id(const QString& id);
   virtual Exception& Component(const QString& component);
   virtual Exception& Severity(Severities severity);
   virtual Exception& Msg(const EventMsg& msg);
   virtual Exception& Arg(int arg, int fieldWidth = 0, Base base = dec, char fillChar = ' ');
   virtual Exception& Arg(unsigned int arg, int fieldWidth = 0, Base base = dec, char fillChar = ' ');
   virtual Exception& Arg(long arg, int fieldWidth = 0, Base base = dec, char fillChar = ' ');
   virtual Exception& Arg(__int64 arg, int fieldWidth = 0, Base base = dec, char fillChar = ' ');
   virtual Exception& Arg(unsigned __int64 arg, int fieldWidth = 0, Base base = dec, char fillChar = ' ');
   virtual Exception& Arg(unsigned long arg, int fieldWidth = 0, Base base = dec, char fillChar = ' ');
   virtual Exception& Arg(double arg);
   virtual Exception& Arg(const QString& arg);
   virtual Exception& Arg(const char* arg);
   virtual Exception& Location(const QString& file, int line);
   virtual Exception& Log();

   virtual Exception* Duplicate();

   void ClearArgs();

public:
   inline QString Id() const { return id; };
   inline QString Component() const { return component; }
   inline QString File() const { return file; }
   inline int Line() const { return line; }

   LocString Info() const;
   LocString FullInfo() const;

protected:
   void FormatInvalidate();
   void Format() const;

   /*! If a derived exception class needs to add any kind of dynamic content
       to #msg before the regular arguments are inserted, it can reimplement
       this PreFormat() method, which returns a LocString that is then used as
       the basis for further formatting.
       \param msg Reference to the const #msg LocString (class member).
       \returns Copy of the passed LocString; modified or not.
       \note The Exception::PreFormat() default implementation just returns an
             unmodified copy of the msg LocString. */
   virtual LocString PreFormat(const LocString& msg) const { return msg; };

   void IntegerBase(Base base, int& baseInt, QString& prefix);

protected:
   QString id;
   QString component;
   EventMsg msg;

   QString file;
   int line;

   QStringList args;

   // these attributes store redundant information, therefore they are mutable
   // this allows to set the methods Info() and FullInfo() as getter methods (const)
   mutable bool formatted;
   mutable EventMsg info;
   mutable EventMsg fullInfo;

};

#endif
