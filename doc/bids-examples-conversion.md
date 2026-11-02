# BIDS example datasets: squirrel conversion status

Generated 2026-11-02 with squirrel build `2026.10.503` against [bids-examples](https://github.com/bids-standard/bids-examples) commit `5e00d47 (2026-10-09)`.

Each dataset was converted with

```
squirrel convert <dataset> <out>.sqrl --inputformat bids --outputformat squirrel
```

and the resulting package was compared with the source BIDS tree: every file under `sub-*/`, `derivatives/` and any other top-level directory (except `sourcedata/`) must be present in the package, subject and session counts must match, and studies must be numbered in session order.

## Summary

| | Datasets |
|---|---|
| ✅ complete | 105 |
| ⚠️ data complete, extras not imported | 3 |
| ❌ incomplete (data files lost) | 0 |
| ❌ conversion failed | 0 |
| **Total** | **108** |

Raw data files kept: **12286 / 12286** (100.0%). Derivative files kept: **2417 / 2417** (100.0%).

### What is being lost

Raw files missing from packages, by kind, across all datasets:

| Kind | Files | Datasets |
|---|---|---|

Derivatives: each `derivatives/<pipeline>/` becomes a pipeline. Each known subject's directory in it (`sub-01`, `sub-01/ses-*`, or `sub-01_ses-pre`, found up to 3 levels deep, eg `freesurfer/subjects/sub-01`) becomes an analysis on the matching study. Outputs that belong to no subject in the dataset (`fsaverage`, template subjects such as `sub-Talairach`, group statistics) become a group-analysis named after the pipeline. A dataset whose root is itself a derivative (`DatasetType: derivative`, eg fMRIPrep output or an atlas) is imported the same way: one pipeline named after `GeneratedBy`, an analysis per subject/session on a `Derived` study, and its other top-level directories (`tpl-*` templates, `logs/`) as a group-analysis.

### Result key

- **✅ complete**: every subject, session and data file made it into the package.
- **⚠️ data complete**: all imaging/recording data and sidecars are present, but `sourcedata/` was not imported (not supported; raw DICOM is better imported with `--inputformat dicom`).
- **❌ incomplete**: one or more data, sidecar or derivative files are missing from the package. This includes root-level sidecars (for example `T1w.json`, `dwi.bval`, `task-*_eeg.json`) that BIDS applies to every matching file.

Not counted as loss: `dataset_description.json`, `participants.tsv`, `sub-*_sessions.tsv`/`.json`, `README`, `CHANGES` and `phenotype/` are parsed into package/subject/study fields and observations instead of being stored as files. `stimuli/` becomes an experiment, `code/` becomes a pipeline named `bids-code`, and other dataset-level files (`LICENSE`, `participants.json`, unused root sidecars, ...) are stored in the package Notes under `bids.files`.

## Datasets

| Dataset | Datatypes | Subjects | Sessions | Series | Raw files kept | Derivative files kept | Result | Notes |
|---|---|---|---|---|---|---|---|---|
| 2d_mb_pcasl | anat, fmap, perf | 1 | 1 | 4 | 9/9 | – | ✅ complete |  |
| 7t_trt | anat, fmap, func | 22 | 44 | 439 | 701/701 | – | ✅ complete |  |
| asl001 | anat, perf | 1 | 1 | 2 | 6/6 | – | ✅ complete |  |
| asl002 | anat, perf | 1 | 1 | 3 | 8/8 | – | ✅ complete |  |
| asl003 | anat, perf | 1 | 1 | 3 | 8/8 | – | ✅ complete |  |
| asl004 | anat, fmap, perf | 1 | 1 | 4 | 10/10 | – | ✅ complete |  |
| asl005 | anat, perf | 1 | 1 | 3 | 8/8 | – | ✅ complete |  |
| atlas-4S | anat, func | 1 | 1 | 0 | 48/48 | – | ⚠️ data complete | sourcedata/ not imported (30 file(s)) |
| atlas-AAL | – | 0 | 0 | 0 | 0/0 | – | ✅ complete |  |
| atlas-Destrieux | – | 0 | 0 | 0 | 0/0 | – | ✅ complete |  |
| atlas-DiFuMo | – | 0 | 0 | 0 | 0/0 | – | ✅ complete |  |
| atlas-HarvardOxford | – | 0 | 0 | 0 | 0/0 | – | ✅ complete |  |
| atlas-HOSPA | – | 0 | 0 | 0 | 0/0 | – | ✅ complete |  |
| atlas-Juelich | – | 0 | 0 | 0 | 0/0 | – | ✅ complete |  |
| atlas-Schaefer | – | 0 | 0 | 0 | 0/0 | – | ✅ complete |  |
| atlas-suit | – | 0 | 0 | 0 | 0/0 | – | ✅ complete |  |
| atlas-Talairach | – | 0 | 0 | 0 | 0/0 | – | ✅ complete |  |
| ds000001-fmriprep | anat, func | 4 | 4 | 0 | 472/472 | – | ✅ complete |  |
| ds000117 | anat, beh, dwi, fmap, func, meg | 17 | 40 | 562 | 957/957 | 539/539 | ✅ complete |  |
| ds000246 | anat, meg | 2 | 2 | 4 | 49/49 | – | ✅ complete |  |
| ds000247 | anat, meg | 6 | 10 | 15 | 197/197 | – | ✅ complete |  |
| ds000248 | anat, meg | 2 | 2 | 5 | 17/17 | 1206/1206 | ✅ complete |  |
| ds001 | anat, func | 16 | 16 | 80 | 128/128 | – | ✅ complete |  |
| ds002 | anat, func | 17 | 17 | 136 | 238/238 | – | ✅ complete |  |
| ds003 | anat, func | 13 | 13 | 39 | 52/52 | – | ✅ complete |  |
| ds004332 | anat | 2 | 2 | 58 | 116/116 | – | ✅ complete |  |
| ds005 | anat, func | 16 | 16 | 80 | 128/128 | – | ✅ complete |  |
| ds006 | anat, func | 14 | 28 | 220 | 384/384 | – | ✅ complete |  |
| ds007 | anat, func | 20 | 20 | 158 | 276/276 | – | ✅ complete |  |
| ds008 | anat, func | 15 | 15 | 113 | 197/197 | – | ✅ complete |  |
| ds009 | anat, func | 24 | 24 | 192 | 360/360 | – | ✅ complete |  |
| ds011 | anat, func | 14 | 14 | 112 | 196/196 | – | ✅ complete |  |
| ds051 | anat, func | 13 | 13 | 143 | 245/245 | – | ✅ complete |  |
| ds052 | anat, func | 13 | 13 | 90 | 142/142 | – | ✅ complete |  |
| ds101 | anat, func | 21 | 21 | 63 | 105/105 | – | ✅ complete |  |
| ds102 | anat, func | 26 | 26 | 78 | 130/130 | – | ✅ complete |  |
| ds105 | anat, func | 6 | 6 | 77 | 148/148 | – | ✅ complete |  |
| ds107 | anat, func | 49 | 49 | 147 | 245/245 | – | ✅ complete |  |
| ds108 | anat, func | 34 | 34 | 238 | 442/442 | – | ✅ complete |  |
| ds109 | anat, func | 36 | 36 | 105 | 174/174 | – | ✅ complete |  |
| ds110 | anat, func | 18 | 18 | 216 | 396/396 | – | ✅ complete |  |
| ds113b | func | 20 | 20 | 156 | 312/312 | – | ✅ complete |  |
| ds114 | anat, dwi, func | 10 | 20 | 140 | 160/160 | – | ✅ complete |  |
| ds116 | anat, func | 17 | 17 | 136 | 238/238 | – | ✅ complete |  |
| ds210 | func | 15 | 15 | 225 | 330/330 | – | ✅ complete |  |
| dwi_deriv | dwi | 1 | 1 | 7 | 16/16 | – | ✅ complete |  |
| eeg_cbm | eeg | 20 | 20 | 20 | 100/100 | – | ✅ complete |  |
| eeg_ds000117 | anat, eeg | 16 | 16 | 112 | 352/352 | – | ✅ complete |  |
| eeg_ds003645s_hed_demo | anat, beh, eeg, micr, motion | 3 | 4 | 30 | 107/107 | – | ✅ complete |  |
| eeg_ds003645s_hed_library | eeg | 2 | 2 | 6 | 24/24 | – | ✅ complete |  |
| eeg_face13 | eeg | 10 | 10 | 10 | 60/60 | – | ⚠️ data complete | sourcedata/ not imported (558 file(s)) |
| eeg_matchingpennies | eeg | 7 | 7 | 7 | 35/35 | – | ⚠️ data complete | sourcedata/ not imported (7 file(s)) |
| eeg_rest_fmri | anat, dwi, eeg, func | 3 | 3 | 18 | 36/36 | 9/9 | ✅ complete |  |
| eeg_rishikesh | eeg | 24 | 40 | 40 | 160/160 | – | ✅ complete |  |
| emg_ConcurrentIndependentUnits | – | 1 | 1 | 2 | 14/14 | – | ✅ complete |  |
| emg_CustomBipolar | – | 1 | 1 | 1 | 3/3 | – | ✅ complete |  |
| emg_CustomBipolarFace | – | 1 | 1 | 1 | 7/7 | – | ✅ complete |  |
| emg_IndependentMod | – | 1 | 1 | 1 | 3/3 | – | ✅ complete |  |
| emg_MultiBodyParts | – | 1 | 1 | 1 | 8/8 | – | ✅ complete |  |
| emg_Multimodal | eeg, motion | 1 | 1 | 3 | 15/15 | – | ✅ complete |  |
| emg_TwoHDsEMG | – | 1 | 1 | 1 | 7/7 | – | ✅ complete |  |
| emg_TwoWristbands | – | 1 | 1 | 1 | 4/4 | – | ✅ complete |  |
| eyetracking_binocular | beh | 1 | 1 | 10 | 14/14 | – | ✅ complete |  |
| eyetracking_eeg_ds007338 | eeg | 1 | 1 | 1 | 12/12 | – | ✅ complete |  |
| eyetracking_fmri | anat, fmap, func | 1 | 1 | 8 | 21/21 | – | ✅ complete |  |
| fnirs_automaticity | nirs | 24 | 24 | 192 | 840/840 | – | ✅ complete |  |
| fnirs_tapping | nirs | 5 | 5 | 5 | 35/35 | – | ✅ complete |  |
| genetics_ukbb | anat, dwi, func | 14 | 14 | 70 | 84/84 | – | ✅ complete |  |
| hcp_example_bids | anat, fmap | 1 | 1 | 5 | 8/8 | – | ✅ complete |  |
| ieeg_epilepsy | anat, ieeg | 1 | 2 | 5 | 27/27 | 13/13 | ✅ complete |  |
| ieeg_epilepsy_ecog | anat, ieeg | 1 | 3 | 4 | 22/22 | 337/337 | ✅ complete |  |
| ieeg_epilepsyNWB | anat, ieeg | 1 | 2 | 5 | 21/21 | 13/13 | ✅ complete |  |
| ieeg_filtered_speech | ieeg | 7 | 7 | 7 | 70/70 | – | ✅ complete |  |
| ieeg_motorMiller2007 | ieeg | 16 | 16 | 16 | 142/142 | 9/9 | ✅ complete |  |
| ieeg_visual | anat, ieeg | 2 | 2 | 5 | 26/26 | 1/1 | ✅ complete |  |
| ieeg_visual_multimodal | anat, fmap, func, ieeg | 1 | 3 | 40 | 141/141 | – | ✅ complete |  |
| micr_SEM | micr | 1 | 2 | 2 | 8/8 | – | ✅ complete |  |
| micr_SEMzarr | micr | 1 | 2 | 3 | 9/9 | – | ✅ complete |  |
| micr_SPIM | micr | 1 | 1 | 8 | 20/20 | – | ✅ complete |  |
| micr_XPCTzarr | micr | 1 | 1 | 1 | 7/7 | – | ✅ complete |  |
| motion_dualtask | eeg, motion | 5 | 10 | 150 | 640/640 | – | ✅ complete |  |
| motion_spotrotation | eeg, motion | 5 | 10 | 25 | 150/150 | – | ✅ complete |  |
| motion_systemvalidation | motion | 3 | 3 | 12 | 39/39 | – | ✅ complete |  |
| mri_chunk | anat | 1 | 1 | 2 | 4/4 | – | ✅ complete |  |
| mrs_2dmrsi | anat, mrs | 8 | 8 | 32 | 64/64 | – | ✅ complete |  |
| mrs_biggaba | anat, mrs | 12 | 12 | 84 | 168/168 | – | ✅ complete |  |
| mrs_fmrs | anat, mrs | 15 | 15 | 75 | 165/165 | – | ✅ complete |  |
| pet001 | anat, pet | 1 | 1 | 2 | 8/8 | – | ✅ complete |  |
| pet002 | anat, pet | 2 | 4 | 8 | 16/16 | – | ✅ complete |  |
| pet003 | anat, pet | 1 | 1 | 2 | 5/5 | – | ✅ complete |  |
| pet004 | pet | 1 | 1 | 1 | 6/6 | – | ✅ complete |  |
| pet005 | anat, pet | 1 | 2 | 4 | 10/10 | – | ✅ complete |  |
| pet006 | pet | 1 | 1 | 1 | 2/2 | – | ✅ complete |  |
| pheno004 | anat | 3 | 2 | 2 | 4/4 | – | ✅ complete | subjects 2 in BIDS vs 3 in package |
| qmri_irt1 | anat | 1 | 1 | 4 | 8/8 | 6/6 | ✅ complete |  |
| qmri_megre | anat | 1 | 1 | 8 | 16/16 | – | ✅ complete |  |
| qmri_mese | anat | 1 | 1 | 32 | 64/64 | 8/8 | ✅ complete |  |
| qmri_mp2rage | anat | 1 | 1 | 7 | 9/9 | 6/6 | ✅ complete |  |
| qmri_mp2rageme | anat, fmap | 1 | 1 | 11 | 17/17 | 10/10 | ✅ complete |  |
| qmri_mpm | anat, fmap | 1 | 1 | 53 | 106/106 | 18/18 | ✅ complete |  |
| qmri_mtsat | anat, fmap | 1 | 1 | 5 | 10/10 | 12/12 | ✅ complete |  |
| qmri_qsm | anat | 1 | 1 | 2 | 4/4 | 4/4 | ✅ complete |  |
| qmri_sa2rage | fmap | 1 | 1 | 2 | 4/4 | 4/4 | ✅ complete |  |
| qmri_tb1tfl | fmap | 1 | 1 | 2 | 4/4 | – | ✅ complete |  |
| qmri_vfa | anat, fmap | 1 | 1 | 4 | 8/8 | 8/8 | ✅ complete |  |
| synthetic | anat, beh, func | 5 | 10 | 50 | 110/110 | 214/214 | ✅ complete |  |
| volume_timing | func | 1 | 1 | 6 | 12/12 | – | ✅ complete |  |
| xeeg_hed_score | anat, eeg, ieeg | 3 | 4 | 11 | 543/543 | – | ✅ complete |  |

## Reproducing

The test harness lives outside the repo. To rerun it: clone `bids-examples`, convert each dataset that has a `dataset_description.json`, unpack each `.sqrl` with `7z`, and compare file names and the `squirrel.json` subject/study/series tree with the source.

