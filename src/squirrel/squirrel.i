%module squirrel

%{
#include <QString>
#include <QDate>
#include <QDateTime>
#include <QDebug>
#include "squirrelSubject.h"
#include "squirrelExperiment.h"
#include "squirrelPipeline.h"
#include "squirrelMeasure.h"
#include "squirrelDrug.h"
#include "squirrelGroupAnalysis.h"
#include "squirrelDataDictionary.h"
#include "squirrelVersion.h"
#include "squirrel.h"
%}

%include "squirrel.h"
