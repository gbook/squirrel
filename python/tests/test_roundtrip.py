import datetime

import pytest

import squirrel


def build_package(path, tmp_path):
    f1 = tmp_path / "t1.nii"
    f1.write_bytes(b"x" * 100)
    f2 = tmp_path / "t2.nii"
    f2.write_bytes(b"y" * 50)

    pkg = squirrel.new(path, name="Test package", description="round trip", data_format="orig")
    s1 = pkg.add_subject("S001", sex="F", date_of_birth=datetime.date(1990, 5, 17), alternate_ids=["A", "B"])
    pkg.add_subject("S002", sex="M")
    st = pkg.add_study(s1, modality="MR", description="baseline", datetime=datetime.datetime(2025, 1, 2, 3, 4, 5))
    pkg.add_series(st, description="T1", protocol="T1w", files=[f1], params={"TR": "2000", "TE": "3"})
    pkg.add_series(st, description="T2", files=[f2])
    pkg.add_observation(s1, "MMSE", value="29", date_start=datetime.datetime(2025, 1, 2))
    pkg.add_intervention(s1, "drug", dose_amount=5.0, date_start=datetime.datetime(2025, 1, 2))
    pkg.add_experiment("exp1")
    pkg.add_pipeline("pipe1", version=2)
    pkg.write()
    return pkg


def test_roundtrip(tmp_path):
    path = tmp_path / "pkg.sqrl"
    build_package(path, tmp_path)
    assert path.exists()

    pkg = squirrel.load(path)
    assert pkg.name == "Test package"
    assert pkg.get_object_count(squirrel.ObjectType.SUBJECT) == 2
    assert pkg.get_object_count(squirrel.ObjectType.SERIES) == 2

    subjects = {s.id: s for s in pkg.get_subject_list()}
    assert set(subjects) == {"S001", "S002"}
    s1 = subjects["S001"]
    assert s1.sex == "F"
    assert s1.date_of_birth == datetime.date(1990, 5, 17)
    assert s1.alternate_ids == ["A", "B"]

    studies = pkg.get_study_list(s1.row_id)
    assert len(studies) == 1
    assert studies[0].modality == "MR"
    assert studies[0].study_number == 1

    series = pkg.get_series_list(studies[0].row_id)
    assert sorted(s.description for s in series) == ["T1", "T2"]
    t1 = next(s for s in series if s.description == "T1")
    assert t1.params["TR"] == "2000"
    t2 = next(s for s in series if s.description == "T2")
    # write() recomputes counts and adds a params.json to every series
    assert sorted(f.rsplit("/", 1)[-1] for f in t2.files) == ["params.json", "t2.nii"]

    assert [o.name for o in pkg.get_observation_list(s1.row_id)] == ["MMSE"]
    assert [i.name for i in pkg.get_intervention_list(s1.row_id)] == ["drug"]
    assert [e.name for e in pkg.get_experiment_list()] == ["exp1"]
    assert [(p.name, p.version) for p in pkg.get_pipeline_list()] == [("pipe1", 2)]
    assert pkg.validate()


def test_extract(tmp_path):
    path = tmp_path / "pkg.sqrl"
    build_package(path, tmp_path)
    pkg = squirrel.load(path)
    out = tmp_path / "out"
    pkg.extract(str(out))
    assert any(p.name == "t1.nii" for p in out.rglob("*"))


def test_extract_errors(tmp_path):
    path = tmp_path / "pkg.sqrl"
    build_package(path, tmp_path)
    pkg = squirrel.load(path)
    pkg.package_path = str(tmp_path / "missing.sqrl")
    with pytest.raises(squirrel.SquirrelError):
        pkg.extract(str(tmp_path / "out"))
    assert not (tmp_path / "out").exists()
    (tmp_path / "junk.sqrl").write_text("not an archive")
    pkg.package_path = str(tmp_path / "junk.sqrl")
    with pytest.raises(squirrel.SquirrelError):
        pkg.extract(str(tmp_path / "out2"))


def test_extract_object(tmp_path):
    path = tmp_path / "pkg.sqrl"
    build_package(path, tmp_path)
    pkg = squirrel.load(path)
    row = pkg.find_subject("S001")
    out = tmp_path / "subj"
    out.mkdir()
    pkg.extract_object(squirrel.ObjectType.SUBJECT, row, str(out))
    assert any(p.name == "t1.nii" for p in out.rglob("*"))


def test_duplicate_subject_raises(tmp_path):
    pkg = squirrel.new(tmp_path / "x.sqrl")
    pkg.add_subject("S001")
    with pytest.raises(squirrel.SquirrelError):
        pkg.add_subject("S001")


def test_invalid_subject_raises(tmp_path):
    pkg = squirrel.new(tmp_path / "x.sqrl")
    with pytest.raises(squirrel.SquirrelError):
        pkg.add_subject("")


def test_unknown_field_rejected(tmp_path):
    pkg = squirrel.new(tmp_path / "x.sqrl")
    with pytest.raises(AttributeError):
        pkg.add_subject("S001", not_a_field=1)


def test_load_missing_file(tmp_path):
    with pytest.raises(squirrel.SquirrelError):
        squirrel.load(tmp_path / "nope.sqrl")


def test_update_after_load(tmp_path):
    path = tmp_path / "pkg.sqrl"
    build_package(path, tmp_path)
    pkg = squirrel.load(path)
    s = pkg.get_subject(pkg.find_subject("S002"))
    s.notes = "edited"
    s.store()
    assert pkg.get_subject(s.row_id).notes == "edited"
