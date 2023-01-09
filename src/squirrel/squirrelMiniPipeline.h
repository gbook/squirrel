/* ------------------------------------------------------------------------------
  Squirrel pipeline.h
  Copyright (C) 2004 - 2022
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


#ifndef SQUIRRELMINIPIPELINE_H
#define SQUIRRELMINIPIPELINE_H
#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

/**
 * @brief squirrelFile
 *
 * This is primarily used by NiDB for inter-instance pipeline sharing but can also be used
 * for other sharing contexts
 */
struct squirrelFile {
    QString filename; /*!< the filename */
    bool executable; /*!< true if file has 777 permissions */
};


/**
 * @brief The squirrelMiniPipeline class
 *
 */
class squirrelMiniPipeline
{
public:
	squirrelMiniPipeline();
	QJsonObject ToJSON();
	void PrintMinipipeline();

    /* minipipeline information (required fields) */
    QString minipipelineName; /*!< minipipeline name (required) */
    QString entryPoint; /*!< name of the file that is the entry point in the minipipeline */

    /* data */
    QList<squirrelFile> files;

private:
    QString virtualPath; /*!< path within the squirrel package, no leading slash */

};

#endif // SQUIRRELMINIPIPELINE_H
