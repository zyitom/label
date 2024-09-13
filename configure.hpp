#ifndef CONFIGURE_HPP
#define CONFIGURE_HPP

#include <QString>

class Configure {
public:
    Configure();
    ~Configure();
    float point_distance = 10;
    float V_rate = 4;
    QString last_open = ".";
    int last_pic = 1;
    int last_mode = 0;
    short auto_enhance_V = 0;
    QString last_model_name = "";
    int last_model_type = 0;
    QString model_path = "";
};

#endif // CONFIGURE_HPP