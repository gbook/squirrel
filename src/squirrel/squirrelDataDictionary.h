/* ------------------------------------------------------------------------------
  Squirrel squirrelDataDictionary.h
  Copyright (C) 2004 - 2023
  Gregory A Book <gregory.book@hhchealth.org> <gregory.a.book@gmail.com>
  Olin Neuropsychiatry Research Center, Hartford Hospital
  ------------------------------------------------------------------------------
  GPLv3 License:

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
  ------------------------------------------------------------------------------ */

#ifndef SQUIRRELDATADICTIONARY_H
#define SQUIRRELDATADICTIONARY_H

#include <QString>

class squirrelDataDictionary
{
public:
    squirrelDataDictionary();

    QString virtualPath; /*!< path within the squirrel package, no leading slash */
};

#endif // SQUIRRELDATADICTIONARY_H