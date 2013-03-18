#include "TrackView.h"
#include "TrackModel.h"
#include <QtGui/QPainter>
#include <QtGui/QKeyEvent>
#include <QtGui/QHeaderView>
#include <iostream>

TrackView::TrackView(QWidget *parent) 
    : QTableView(parent)
{
    trackModel = new TrackModel(this);

    setModel(trackModel);

    QFontMetrics fm(font());

    /* The padding of 4 in "lineSpacing() + 4" has been found by
     * trial-and-error. Do not lower it, but increasing it is fine. */
    verticalHeader()->setDefaultSectionSize(fm.lineSpacing() + 4);
    horizontalHeader()->setDefaultSectionSize(fm.averageCharWidth() * 16);
    verticalHeader()->setResizeMode(QHeaderView::Fixed);

    // setup scrolling to fixate on the current position
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    this->scrollToTop();

    connect(trackModel, SIGNAL(cellChanged(std::string,SyncKey)), this, SIGNAL(cellChanged(std::string,SyncKey)));
}

int TrackView::getCurrentRow() {
    QItemSelectionModel *selection = selectionModel();
    QModelIndex index = selection->currentIndex();
    return index.row();
}

void TrackView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_I) {
        // Change interpolation mode
        QItemSelectionModel *selection = selectionModel();
        QModelIndex index = selection->currentIndex();
        trackModel->changeInterpolationType(index);
        SyncKey key = trackModel->getPrevKey(index);
        emit interpolationTypeChanged(trackModel->getTrackName(index.column()), key);
        this->update(index);
        this->viewport()->update();
        repaint();

    } else if (event->key() == Qt::Key_Space) {
        emit pauseTriggered();
    } else if (event->key() == Qt::Key_Delete) {
        QItemSelectionModel *selection = selectionModel();
        QModelIndex index = selection->currentIndex();
        if (trackModel->isKeyFrame(index)) {
            SyncKey key = trackModel->getExactKey(index);
            trackModel->deleteKey(index);
            emit deleteKey(trackModel->getTrackName(index.column()), key);
        }
    } else {
        QTableView::keyPressEvent(event);
    }
}

void TrackView::resizeEvent(QResizeEvent *event)
{
    scrollTo(selectionModel()->currentIndex(), QAbstractItemView::PositionAtCenter);
    QTableView::resizeEvent(event);
}

void TrackView::createTrack(std::string name)
{
    trackModel->createTrack(name);
    repaint();
}

SyncTrack* TrackView::getTrack(std::string name)
{
    return trackModel->getTrack(name);
}

int TrackView::getTrackIndex(std::string name)
{
    return trackModel->getTrackIndex(name);
}

void TrackView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    QTableView::currentChanged(current, previous);
    emit rowChanged(current.row());
}

void TrackView::changeRow(int row)
{
    setSelectionBehavior(QAbstractItemView::SelectItems);
    setSelectionMode(QAbstractItemView::SingleSelection);
    QItemSelectionModel *selection = selectionModel();
    QModelIndex select = selection->currentIndex();
    QModelIndex index = trackModel->index(row, select.column());
    selection->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
}
