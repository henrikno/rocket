#include "TrackModel.h"
#include "SyncTrack.h"

TrackModel::TrackModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    numRows = 12800;
}

int TrackModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return numRows; //top-level: return list size
    else
        return 0; //list item: no further children (flat list)
}

int TrackModel::columnCount(const QModelIndex & /* parent */) const
{
    return tracks.size();
}

Qt::ItemFlags TrackModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

QVariant TrackModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (index.column() >= tracks.size())
        return QVariant();

    SyncTrack *track = tracks[index.column()];

    QString str;
    QVariant var(QVariant::Double);
    switch (role) {
    case Qt::DisplayRole:
        if (track->isKeyFrame(index.row()))
            return str.setNum(track->getValue(index.row()));
        return QVariant("---");

    case Qt::BackgroundRole:
        if (track->getKeyMap().size() && index.row() >= track->getKeyMap().begin()->first) {
            if (track->getPrevKey(index.row()).type == SyncKey::LINEAR)
                return QBrush(QColor(0, 255, 0, 16));
        }

        if (index.row() % 8 == 0)
            return QBrush(QColor(0, 0, 0, 16));
        return QVariant();

    case Qt::EditRole:
        if (track->isKeyFrame(index.row())) {
            var.setValue(track->getValue(index.row()));
            return var.toDouble();
        }
        return QVariant(QVariant::Double);

    case Qt::TextColorRole:
        if (track->isKeyFrame(index.row())) {
            return QBrush(QColor(0, 0, 0, 255));
        }
        return QBrush(QColor(64, 64, 64, 32));
    }

    return QVariant();
}

bool TrackModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.column() >= tracks.size())
        Q_ASSERT(false);

    bool ok;
    value.toFloat(&ok);

    SyncKey key;
    key.row = index.row();
    key.type = SyncKey::STEP;
    key.value = value.toFloat();
    tracks[index.column()]->setKey(key);

    emit cellChanged(tracks[index.column()]->getName(), key);

    dataChanged(index, index);
    return true;
}


QVariant TrackModel::headerData(int section,
                                Qt::Orientation orientation,
                                int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            if (section >= tracks.size()) return QVariant();
            return QVariant(QString(tracks[section]->getName().c_str()));
        }
        else
            return QVariant(QString("%1").arg(section, 4, 16, QLatin1Char('0')));
    }
    if (role == Qt::BackgroundRole) {
        return QBrush(QColor(255,0,0,16));
    }

    return QVariant();
}

SyncKey TrackModel::getPrevKey(const QModelIndex &index) {
    SyncKey key = {};
    if (index.column() >= tracks.size()) return key;

    return tracks[index.column()]->getPrevKey(index.row());
}

SyncKey TrackModel::getExactKey(const QModelIndex &index) {
    SyncKey key = {};
    if (index.column() >= tracks.size()) return key;

    return tracks[index.column()]->getExactKey(index.row());
}

bool TrackModel::isKeyFrame(const QModelIndex &index)
{
    if (index.column() >= tracks.size())
        return false;

    return tracks[index.column()]->isKeyFrame(index.row());
}

void TrackModel::deleteKey(const QModelIndex &index)
{
    tracks[index.column()]->delKey(index.row());
    dataChanged(index, index);
}

void TrackModel::changeInterpolationType(const QModelIndex &index)
{
    if (index.column() >= tracks.size() || !tracks[index.column()]->getKeyMap().size())
        return;

    SyncKey key = tracks[index.column()]->getPrevKey(index.row());
    key.type = static_cast<SyncKey::Type>((key.type+1)%4);

    tracks[index.column()]->setKey(key);

}

bool TrackModel::hasTrack(std::string name) {
    for (size_t i = 0; i < tracks.size(); ++i) {
        if (tracks[i]->getName() == name) {
            return true;
        }
    }
    return false;}

void TrackModel::createTrack(std::string name)
{
    bool found = hasTrack(name);
    if (found) return;

    int n = tracks.size();
    beginInsertColumns(QModelIndex(), n,n);
    columnOrder.push_back(columnOrder.size());
    tracks.push_back(new SyncTrack(name));
    endInsertColumns();
}

SyncTrack * TrackModel::getTrack(std::string name)
{
    for (size_t i = 0; i < tracks.size(); ++i) {
        if (tracks[i]->getName() == name) {
            return tracks[i];
        }
    }
    return NULL;
}

int TrackModel::getTrackIndex(std::string name)
{
    for (size_t i = 0; i < tracks.size(); ++i) {
        if (tracks[i]->getName() == name) {
            return columnOrder[i];
        }
    }
    return -1;
}

std::string TrackModel::getTrackName(int column)
{
    if (column >= tracks.size()) return "";
    return tracks[column]->getName();
}

