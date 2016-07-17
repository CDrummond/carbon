/*
  Carbon (C) Craig Drummond, 2013 craig.p.drummond@gmail.com

  ----

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; see the file COPYING.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "excludewidget.h"
#include "excludefile.h"
#include "session.h"
#include <QIcon>
#include <QCheckBox>
#include <QSettings>
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>
#include <unistd.h>
#include <sys/types.h>

#define CFG_GROUP     "ExcludeWidget/"
#define CFG_COL_SIZES CFG_GROUP "List"

ExcludeWidget::ExcludeWidget(QWidget *parent)
    : QWidget(parent) {
    setupUi(this);

    addButton->setText(tr("Add"));
    addButton->setToolTip(tr("Add an exclude pattern."));
    addButton->setIcon(QIcon::fromTheme("list-add"));
    removeButton->setText(tr("Delete"));
    removeButton->setToolTip(tr("Delete the selected exclude patterns."));
    removeButton->setIcon(QIcon::fromTheme("list-remove"));

    connect(addButton, SIGNAL(clicked()), SLOT(add()));
    connect(removeButton, SIGNAL(clicked()), SLOT(remove()));
    connect(excludeList, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), SLOT(editItem(QTreeWidgetItem *, int)));
    connect(excludeList, SIGNAL(itemSelectionChanged()), SLOT(controlButtons()));

    QStringList list;
    QSettings cfg;
    list = cfg.value(CFG_COL_SIZES, list).toStringList();

    if (2 == list.count()) {
        for (int i = 0; i < 2; ++i) {
            excludeList->header()->resizeSection(i, list[i].toInt());
        }
    }

    excludeList->sortItems(0, Qt::AscendingOrder);
}

ExcludeWidget::~ExcludeWidget() {
    QStringList list;
    for (int i = 0; i < 2; ++i) {
        list << QString::number(excludeList->header()->sectionSize(i));
    }

    QSettings cfg;
    cfg.setValue(CFG_COL_SIZES, list);
}

void ExcludeWidget::set(const Session &session) {
    cvsExclude->setChecked(session.cvsExcludeFlag());
    maxSize->setValue(session.maxSize());
    excludeList->clear();
    if (session.excludeFile()) {
        foreach (const ExcludeFile::Pattern &p, session.excludeFile()->patterns()) {
            addPattern(p.value, p.comment);
        }
    }
    controlButtons();
}

void ExcludeWidget::get(Session &session) {
    session.setCvsExcludeFlag(cvsExclude->isChecked());
    session.setMaxSize(maxSize->value());

    ExcludeFile::PatternList list;
    for (int i = 0; i < excludeList->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = excludeList->topLevelItem(i);
        list.append(ExcludeFile::Pattern(item->text(0), item->text(1)));
    }
    session.setExcludePatterns(list);
}

void ExcludeWidget::addPattern(const QString &v, const QString &c) {
    QStringList list;

    list << v;
    if (!c.isEmpty()) {
        list << c;
    }

    QTreeWidgetItem *i = new QTreeWidgetItem(excludeList, list);
    i->setFlags(i->flags() | Qt::ItemIsEditable);
}

void ExcludeWidget::add() {
    QString items(QInputDialog::getText(this, tr("New Exclude Patterns"),
                                        tr("<p>Please enter a comma separated list of patterns "
                                           "to exclude.</p><p><i>e.g. *.sav, *.inf</i></p>")).trimmed());

    if (!items.isEmpty()) {
        QStringList patterns(items.split(',', QString::SkipEmptyParts));
        QStringList::ConstIterator it(patterns.begin()),
                    end(patterns.end());

        for (; it != end; ++it) {
            QString v((*it).trimmed());

            if (0 == excludeList->findItems(v, Qt::MatchExactly, 0).count()) {
                addPattern(v);
            }
        }
    }
}

void ExcludeWidget::remove() {
    QList<QTreeWidgetItem *> items(excludeList->selectedItems());

    if (items.count() && QMessageBox::Yes == QMessageBox::warning(this, tr("Delete"), tr("Delete all the selected patterns?"), QMessageBox::Yes | QMessageBox::No)) {
        foreach (QTreeWidgetItem *i, items) {
            delete i;
        }

        controlButtons();
    }
}

void ExcludeWidget::controlButtons() {
    removeButton->setEnabled(excludeList->selectedItems().count());
}

void ExcludeWidget::editItem(QTreeWidgetItem *item, int col) {
    excludeList->editItem(item, col);
}
