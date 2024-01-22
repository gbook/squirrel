CREATE TABLE Analysis (
  AnalysisRowID INTEGER PRIMARY KEY,
  StudyRowID INTEGER,
  PipelineRowID INTEGER,
  PipelineVersion INTEGER,
  StartDate TEXT,
  EndDate TEXT,
  SetupTime INTEGER,
  RunTime INTEGER,
  NumSeries INTEGER,
  Status TEXT,
  Successful INTEGER,
  Size INTEGER,
  Hostname TEXT,
  StatusMessage TEXT,
  VirtualPath TEXT 
)

CREATE TABLE Drug (
  DrugRowID INTEGER PRIMARY KEY,
  SubjectRowID INTEGER,
  DrugName TEXT,
  DateStart TEXT,
  DateEnd TEXT,
  DateRecordEntry TEXT,
  DoseString TEXT,
  DoseAmount TEXT,
  DoseFrequency TEXT,
  AdministrationRoute TEXT,
  DrugClass TEXT,
  DoseKey TEXT,
  DoseUnit TEXT,
  FrequencyModifer TEXT,
  FrequencyValue REAL,
  FrequencyUnit TEXT,
  Description TEXT,
  Rater TEXT,
  Notes TEXT 
)

CREATE TABLE Experiment (
  ExperimentRowID INTEGER PRIMARY KEY,
  PackageRowID INTEGER,
  ExperimentName TEXT,
  Size INTEGER ,
  NumFiles INTEGER ,
  VirtualPath TEXT 
)

CREATE TABLE GroupAnalysis (
  GroupAnalysisRowID INTEGER PRIMARY KEY,
  PackageRowID INTEGER,
  Name TEXT,
  Description TEXT,
  Datetime TEXT,
  NumFiles INTEGER,
  Size INTEGER,
  VirtualPath TEXT 
)

CREATE TABLE Measure (
  MeasureRowID INTEGER PRIMARY KEY,
  SubjectRowID INTEGER,
  MeasureName TEXT,
  DateStart TEXT,
  DateEnd TEXT,
  InstrumentName TEXT,
  Rater TEXT,
  Notes TEXT,
  Value TEXT,
  Description TEXT 
)

CREATE TABLE Package (
  PackageRowID INTEGER PRIMARY KEY,
  Name TEXT,
  Description TEXT,
  Datetime TEXT,
  SubjectDirFormat TEXT,
  StudyDirFormat TEXT,
  SeriesDirFormat TEXT,
  PackageDataFormat TEXT,
  License TEXT,
  Readme TEXT,
  Changes TEXT,
  Notes TEXT 
)

CREATE TABLE Params (
  ParamRowID INTEGER PRIMARY KEY,
  SeriesRowID INTEGER,
  ParamKey TEXT,
  ParamValue TEXT 
)

CREATE TABLE Pipeline (
  PipelineRowID INTEGER PRIMARY KEY,
  PackageRowID INTEGER,
  Name TEXT,
  Description TEXT,
  Datetime TEXT,
  Level INTEGER,
  PrimaryScript TEXT,
  SecondaryScript TEXT,
  Version INTEGER ,
  CompleteFiles TEXT,
  DataCopyMethod TEXT,
  DependencyDirectory TEXT,
  DependencyLevel TEXT,
  DependencyLinkType TEXT,
  DirStructure TEXT,
  Directory TEXT,
  GroupName TEXT,
  GroupType TEXT,
  Notes TEXT,
  ResultScript TEXT,
  TempDir TEXT,
  FlagUseProfile INTEGER,
  FlagUseTempDir INTEGER,
  ClusterType TEXT,
  ClusterUser TEXT,
  ClusterQueue TEXT,
  ClusterSubmitHost TEXT,
  NumConcurrentAnalysis INTEGER,
  MaxWallTime INTEGER,
  SubmitDelay INTEGER,
  VirtualPath TEXT 
)

CREATE TABLE PipeplineDataStep (
  DataStepRowID INTEGER PRIMARY KEY,
  PipelineRowID INTEGER,
  AssociationType TEXT,
  BehDir TEXT,
  BehFormat TEXT,
  DataFormat TEXT,
  ImageType TEXT,
  DataLevel TEXT,
  Location TEXT,
  Modality TEXT,
  NumBOLDReps TEXT,
  NumImagesCriteria TEXT,
  StepOrder INTEGER,
  Protocol TEXT,
  SeriesCriteria TEXT,
  FlagEnabled INTEGER,
  FlagOptional INTEGER,
  FlagGzip INTEGER,
  FlagPreserveSeriesNum INTEGER,
  FlagPrimaryProtocol INTEGER,
  FlagUsePhaseDir INTEGER,
  FlagUseSeries INTEGER 
)

CREATE TABLE Series (
  SeriesRowID INTEGER PRIMARY KEY,
  StudyRowID INTEGER,
  SeriesNum INTEGER,
  Datetime TEXT,
  SeriesUID TEXT,
  Description TEXT,
  Protocol TEXT,
  ExperimentRowID INTEGER,
  Size INTEGER,
  NumFiles INTEGER,
  BehSize INTEGER,
  BehNumFiles INTEGER,
  VirtualPath TEXT 
)

CREATE TABLE Study (
  StudyRowID INTEGER PRIMARY KEY,
  SubjectRowID INTEGER,
  StudyNumber INTEGER ,
  Datetime TEXT,
  Age double,
  Height double,
  Weight double,
  Modality TEXT,
  Description TEXT,
  StudyUID TEXT,
  VisitType TEXT,
  DayNumber INTEGER,
  Timepoint INTEGER,
  Equipment TEXT,
  VirtualPath TEXT 
)

CREATE TABLE Subject (
  SubjectRowID INTEGER PRIMARY KEY,
  PackageRowID INTEGER,
  ID TEXT,
  AltIDs TEXT,
  GUID TEXT,
  DateOfBirth date,
  Sex TEXT,
  Gender TEXT,
  Ethnicity1 TEXT,
  Ethnicity2 TEXT,
  VirtualPath TEXT 
)
