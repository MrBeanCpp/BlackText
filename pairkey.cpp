#include "pairkey.h"
const QMap<int, QPair<int, QString>> PairKey::map = { //static必须定义在Cpp内
    { Qt::Key_ParenLeft, { Qt::Key_ParenRight, "()" } },
    { Qt::Key_BraceLeft, { Qt::Key_BraceRight, "{}" } },
    { Qt::Key_BracketLeft, { Qt::Key_BracketRight, "[]" } },
    { Qt::Key_QuoteDbl, { Qt::Key_QuoteDbl, "\"\"" } },
    { Qt::Key_Apostrophe, { Qt::Key_Apostrophe, "\'\'" } }
};
PairKey::PairKey()
{
}

int PairKey::getRight(int leftKey)
{
    if (!find(leftKey))
        return NOFIND;
    else
        return map.value(leftKey).first;
}

QString PairKey::getStr(int leftKey)
{
    if (!find(leftKey))
        return "NOFIND";
    else
        return map.value(leftKey).second;
}

bool PairKey::find(int leftKey)
{
    return map.find(leftKey) != map.end();
}
