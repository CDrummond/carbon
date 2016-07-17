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

#include "treewidget.h"
#include "basicitemdelegate.h"
#include <QApplication>
#include <QPainter>
#include <QImage>

static QImage setOpacity(const QImage &orig, double opacity) {
    QImage img = QImage::Format_ARGB32 == orig.format() ? orig : orig.convertToFormat(QImage::Format_ARGB32);
    uchar *bits = img.bits();
    for (int i = 0; i < img.height()*img.bytesPerLine(); i += 4) {
        if (0 != bits[i + 3]) {
            bits[i + 3] = opacity * 255;
        }
    }
    return img;
}

static QPixmap createBgndPixmap(const QIcon &icon) {
    if (icon.isNull()) {
        return QPixmap();
    }
    static int bgndSize = 64;

    QImage img = icon.pixmap(bgndSize, bgndSize).toImage();
    if (img.width() != bgndSize && img.height() != bgndSize) {
        img = img.scaled(bgndSize, bgndSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    img = setOpacity(img, 0.075);
    QPixmap pix(bgndSize * 2, bgndSize * 2);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.drawImage(bgndSize / 2, bgndSize / 2, img);
    p.end();
    return pix;
}

TreeWidget::TreeWidget(QWidget *p)
    : QTreeWidget(p) {
    setAlternatingRowColors(false);
    setRootIsDecorated(false);
    setSortingEnabled(true);
    setAllColumnsShowFocus(true);
    setIndentation(0);
    setItemDelegate(new BasicItemDelegate(this));
}

void TreeWidget::setBackground(const QIcon &icon) {
    QPalette pal = parentWidget()->palette();
    if (!icon.isNull()) {
        pal.setColor(QPalette::Base, Qt::transparent);
    }
    setPalette(pal);
    viewport()->setPalette(pal);
    bgnd = createBgndPixmap(icon);
}

void TreeWidget::paintEvent(QPaintEvent *e) {
    if (!bgnd.isNull()) {
        QPainter p(viewport());
        p.fillRect(viewport()->rect(), bgnd);
    }
    QTreeWidget::paintEvent(e);
}
