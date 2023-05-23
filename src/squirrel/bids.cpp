/* ------------------------------------------------------------------------------
  Squirrel bids.cpp
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



#include "bids.h"


/* ---------------------------------------------------------------------------- */
/* ----- bids ----------------------------------------------------------------- */
/* ---------------------------------------------------------------------------- */
bids::bids()
{

}


/* ---------------------------------------------------------------------------- */
/* ----- LoadToSquirrel ------------------------------------------------------- */
/* ---------------------------------------------------------------------------- */
bool LoadToSquirrel(QString dir, squirrel *sqrl, QString msg) {

    QStringList msgs;

    /* check if directory exists */
    QDir d(dir);
    if (!d.exists()) {
        msgs << QString("Directory [%1] does not exist").arg(dir);
        msg = msgs.join("\n");
        return false;
    }


    /* check for all .json files in the root directory, read into consolidated JSON object
       call it bids.json, with each file in it's own object ? */
    QStringList rootfiles = FindAllFiles(dir, "*", false);

    /* 2 - get list of sub-* directories and read participants.tsv */
    foreach (QString rootfile, rootfiles) {
        QFileInfo rootfi(rootfile);

        /* 3 - for each subject, read all of the ses-* directories and read sessions.tsv */
        if (rootfi.isDir() && (rootfi.fileName().startsWith("sub-"))) {
            QStringList subfiles = FindAllFiles(rootfi.absoluteFilePath(), "*", false);

            /* 4 - for each session, read directories and scans.tsv */

                /* for each scan... */
                    /* map the BIDS thing to an actual modality: MapBIDStoModality() */
                    /* parse the file names --> protocol and run/series number */
                    /* read the .json file for all the parameters --> params.json, modality, maybe other info */
                    /* the real modalitity might be in one of the .json files */
    }

    /* check for a 'derivatives' directory, which are analyses */
}
