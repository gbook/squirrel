/* ------------------------------------------------------------------------------
  Squirrel GUI mergeDialog.h
  Copyright (C) 2004 - 2026
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

#ifndef MERGEDIALOG_H
#define MERGEDIALOG_H

#include <QDialog>
#include <QStringList>

namespace Ui { class mergeDialog; }

/* ------------------------------------------------------------------------------
   mergeDialog

   Collects the arguments for modify::MergePackages(). It only gathers and
   validates input - the merge itself is run by squirrelWorker on its own thread.
   ------------------------------------------------------------------------------ */
class mergeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit mergeDialog(QWidget *parent = nullptr);
    ~mergeDialog();

    QStringList InputPaths() const;
    QString OutputPath() const;
    bool TestOnly() const;
    bool RenumberSubjects() const;
    int Digits() const;

protected:
    void accept() override;

private slots:
    void AddPackages();
    void RemoveSelected();
    void MoveUp();
    void MoveDown();
    void BrowseOutput();
    void UpdateWidgetStates();

private:
    Ui::mergeDialog *ui;
};

#endif // MERGEDIALOG_H
