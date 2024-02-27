#ifndef PAIRKEY_H
#define PAIRKEY_H
#include <QMap>
#include <QPair>
#include <QString>
class PairKey {
public:
    PairKey();
    static int getRight(int leftKey);
    static QString getStr(int leftKey);
    static bool find(int leftKey);

private:
    static const QMap<int, QPair<int, QString>> map;
    enum {
        NOFIND
    };
};

#endif // PAIRKEY_H
