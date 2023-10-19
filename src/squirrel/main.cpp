/* ------------------------------------------------------------------------------
  Squirrel main.cpp
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

#include <QCoreApplication>
#include <QCommandLineParser>
#include <iostream>
#include "squirrelVersion.h"
#include "validate.h"
#include "dicom.h"
#include "bids.h"
#include "squirrel.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    /* this whole section reads the command line parameters */
    a.setApplicationVersion(QString("%1.%2").arg(SQUIRREL_VERSION_MAJ).arg(SQUIRREL_VERSION_MIN));
    a.setApplicationName("Squirrel");

    /* setup the command line parser */
    QCommandLineParser p;
    p.setApplicationDescription("Squirrel data format tools");
    p.setSingleDashWordOptionMode(QCommandLineParser::ParseAsCompactedShortOptions);
    p.setOptionsAfterPositionalArgumentsMode(QCommandLineParser::ParseAsOptions);
    p.addHelpOption();
    p.addVersionOption();
    p.addPositionalArgument("tool", "Available tools:  validate  dicom2squirrel  bids2squirrel  squirrel2bids  modify  list");

    /* command line flag options */
    QCommandLineOption optDebug(QStringList() << "d" << "debug", "Enable debugging");
    QCommandLineOption optQuiet(QStringList() << "q" << "quiet", "Dont print headers and checks");
    p.addOption(optDebug);
    p.addOption(optQuiet);

    /* command line options that take values */
    QCommandLineOption optInputFile(QStringList() << "i" << "in", "Input file/directory", "input file/dir");
    QCommandLineOption optOutputFile(QStringList() << "o" << "out", "Output file/directory", "output file/dir");
    QCommandLineOption optOutputDataFormat(QStringList() << "output-data-format", "Output data format, if converted from DICOM:\n  anon - Anonymized DICOM\n  nifti4d - Nifti 4D\n  nifti4dgz - Nifti 4D gz (default)\n  nifti3d - Nifti 3D\n  nifti3dgz - Nifti 3D gz", "outputdataformat");
    QCommandLineOption optOutputDirFormat(QStringList() << "output-dir-format", "Output directory structure\n  seq - Sequentially numbered\n  orig - Original ID (default)", "outputdirformat");
    QCommandLineOption optOutputPackageFormat(QStringList() << "output-package-format", "Output package format\n  dir - Directory\n  zip - .zip file (default)", "outputpackageformat");
    QCommandLineOption optRenumberIDs(QStringList() << "renumber-ids", "Renumber IDs in zero-padded format #####. Existing IDs are moved to subject alt-IDs field");
    QCommandLineOption optListSubjects(QStringList() << "list-subjects", "Print a list of subjects");
    QCommandLineOption optListStudies(QStringList() << "list-studies", "Print a list of studies for a subject", "subjectid");
    QCommandLineOption optListSeries(QStringList() << "list-series", "Print a list series for a study", "subjectid,studynum");
    QCommandLineOption optListDrugs(QStringList() << "list-drugs", "Print a list of drugs for a subject", "subjectid");
    QCommandLineOption optListMeasures(QStringList() << "list-measures", "Print a list of measures for a subject", "subjectid");
    QCommandLineOption optListExperiments(QStringList() << "list-experiments", "Print a list of experiments");
    QCommandLineOption optListPipelines(QStringList() << "list-pipelines", "Print a list of pipelines");
    QCommandLineOption optListGroupAnalyses(QStringList() << "list-groupanalyses", "Print a list of group analyses");
    QCommandLineOption optListDataDictionary(QStringList() << "list-datadictionary", "Print the data dictionary");
    QCommandLineOption optListDetails(QStringList() << "list-details", "Include details when printing lists");
    p.addOption(optOutputFile);
    p.addOption(optInputFile);
    p.addOption(optOutputDataFormat);
    p.addOption(optOutputDirFormat);
    p.addOption(optOutputPackageFormat);
    p.addOption(optRenumberIDs);
    p.addOption(optListSubjects);
    p.addOption(optListStudies);
    p.addOption(optListSeries);
    p.addOption(optListDetails);

    p.addOption(optListDrugs);
    p.addOption(optListMeasures);
    p.addOption(optListExperiments);
    p.addOption(optListPipelines);
    p.addOption(optListGroupAnalyses);
    p.addOption(optListDataDictionary);

    /* Process the actual command line arguments given by the user */
    p.process(a);

    QString tool;
    bool debug, quiet;
    bool renumberIDs;
    bool listDetails;
    bool listSubjects;

    const QStringList args = p.positionalArguments();
    if (args.size() > 0)
        tool = args.at(0).trimmed();

    debug = p.isSet(optDebug);
    quiet = p.isSet(optQuiet);
    QString paramOutputFile = p.value(optOutputFile).trimmed();
    QString paramInput = p.value(optInputFile).trimmed();
    QString paramOutputDataFormat = p.value(optOutputDataFormat).trimmed();
    QString paramOutputDirFormat = p.value(optOutputDirFormat).trimmed();
    QString paramOutputPackageFormat = p.value(optOutputPackageFormat).trimmed();
    listSubjects = p.isSet(optListSubjects);
    QString paramSubjectID = p.value(optListStudies).trimmed();
    QString paramSubjectIDStudyNum = p.value(optListSeries).trimmed();
    renumberIDs = p.isSet(optRenumberIDs);
    listDetails = p.isSet(optListDetails);

    QStringList tools = { "dicom2squirrel", "validate", "bids2squirrel", "squirrel2bids", "modify", "list" };

    /* now check the command line parameters passed in, to see if they are calling a valid module */
    if (!tools.contains(tool)) {
        if (tool != "")
            std::cout << QString("***** Error: unrecognized option [%1] *****\n").arg(tool).toStdString().c_str();

        std::cout << p.helpText().toStdString().c_str();
        return 0;
    }

    QString bindir = QDir::currentPath();

    if (!quiet) {
        Print("+----------------------------------------------------+");
        Print(QString("|  Squirrel utils version %1.%2\n|\n|  Build date [%3 %4]\n|  C++ [%5]\n|  Qt compiled [%6]\n|  Qt runtime [%7]\n|  Build system [%8]" ).arg(SQUIRREL_VERSION_MAJ).arg(SQUIRREL_VERSION_MIN).arg(__DATE__).arg(__TIME__).arg(__cplusplus).arg(QT_VERSION_STR).arg(qVersion()).arg(QSysInfo::buildAbi()));
        Print(QString("|\n|  Current working directory is %1").arg(bindir));
        Print("+----------------------------------------------------+\n");
    }

    /* ---------- Run the validate tool ---------- */
    if (tool == "validate") {
        if (paramInput.trimmed() == "") {
            Print("*** Input file blank ***");
        }
        else if (!QFile::exists(paramInput)) {
            Print(QString("*** Input file [%1] does not exist ***").arg(paramInput));
        }
        else {
            /* create squirrel object and validate */
            squirrel *sqrl = new squirrel(debug);
            if (sqrl->read(paramInput, true)) {
                sqrl->Log("Valid squirrel file", __FUNCTION__);
            }
            else {
                sqrl->Log("*** Invalid squirrel file ***", __FUNCTION__);
            }
        }
    }
    /* ---------- Run the dicom2squirrel tool ---------- */
    else if (tool == "dicom2squirrel") {

        /* check if the outfile's parent directory exists */
        QFileInfo outinfo(paramOutputFile);
        QDir outdir = outinfo.absolutePath();
        if (!outdir.exists()) {
            Print(QString("Output directory [%1] does not exist").arg(outdir.absolutePath()));
        }
        else {
            dicom *dcm = new dicom();
            squirrel *sqrl = new squirrel(debug);
            sqrl->dataFormat = paramOutputDataFormat;
            sqrl->subjectDirFormat = paramOutputDirFormat;
            sqrl->studyDirFormat = paramOutputDirFormat;
            sqrl->seriesDirFormat = paramOutputDirFormat;
            sqrl->packageFormat = paramOutputPackageFormat;

            /* 1) load the DICOM data to a squirrel object */
            dcm->LoadToSquirrel(paramInput, bindir, sqrl);

            /* 2) write the squirrel file */
            QString filepath;
            sqrl->write(paramOutputFile, filepath);

            delete dcm;
            delete sqrl;
        }

    }
    /* ---------- Run the bids2squirrel tool ---------- */
    else if (tool == "bids2squirrel") {

        Print(QString("Running bids2squirrel on input directory [%1]").arg(paramInput));
        Print(QString("Output file [%1]").arg(paramOutputFile));

        /* check if the infile directory exists */
        QDir indir(paramInput);
        if (!indir.exists()) {
            Print(QString("Input directory [%1] does not exist").arg(indir.absolutePath()));
        }
        else if (paramInput == "") {
            Print("Input directory not specified. Use the -i <indir> option to specify the input directory");
        }
        else {
            QString outputfile = paramOutputFile;

            if (paramOutputFile == "") {
                outputfile = QString(paramInput + "/squirrel.zip");
                Print(QString("Output file not specified. Creating squirrel file in input directory [%1]").arg(outputfile));
            }

            /* create a squirrel object */
            squirrel *sqrl = new squirrel(debug);

            /* create a BIDS object, and start reading the directory */
            bids *bds = new bids();

            bds->LoadToSquirrel(indir.path(), sqrl);

            /* display progress or messages */

            /* save the squirrel object */
            QString outpath;
            sqrl->filePath = outputfile;
            //sqrl->print();
            sqrl->write(paramInput, outpath);
        }
    }
    /* ---------- Run the squirrel2bids tool ---------- */
    else if (tool == "squirrel2bids") {

    }
    /* ---------- Run the list tool ---------- */
    else if (tool == "list") {
        /* check if the infile exists */
        QFile infile(paramInput);
        if (!infile.exists()) {
            Print(QString("Input file [%1] does not exist").arg(paramInput));
        }
        else {
            squirrel *sqrl = new squirrel(debug);
            sqrl->read(paramInput);

            if (listSubjects) {
                /* print list of all subjects */
                sqrl->PrintSubjects(listDetails);
            }
            else if (paramSubjectID != "") {
                /* print list of studies for this subjectID */
                sqrl->PrintStudies(paramSubjectID, listDetails);
            }
            else if (paramSubjectIDStudyNum != "") {
                QStringList parts = paramSubjectIDStudyNum.split(",");
                if (parts.size() == 2) {
                    QString subjectID = parts[0];
                    int studyNum = parts[1].toInt();
                    /* print list of studies for this subjectID, studynum */
                    sqrl->PrintSeries(subjectID, studyNum, listDetails);
                }
                else {
                    Print("Incorrect argument " + paramSubjectIDStudyNum + " for parameter --list-series");
                }
            }

            delete sqrl;
        }
    }

    Print("\nExiting squirrel utils");
    a.exit();
    return 0;
}
