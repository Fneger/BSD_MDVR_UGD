#include "st_message.h"

namespace BdUgdServer{

QString GetCustom(const QString name)
{
    QString custom;
    QStringList strs = name.split("-");
    if(strs.size() == 5)
    {
        strs = strs.at(4).split(".");
        custom = strs.first();
    }
    else
        custom = "STD";
    return custom;
}

};
