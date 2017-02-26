//
// Created by garrus on 24.07.16.
//

#include "FootTargetFunctor.h"

using namespace StepsController;

FootTargetFunctor::FootTargetFunctor(boost::shared_ptr<float[]> normal_angles, pcl::PointCloud<pcl::PointXYZRGB>::Ptr  cloud, StepsParams step_params, float step) :
        angle_func(normal_angles, cloud->width, cloud->height),
        cloud_func(cloud)
{
    this->normal_angles = normal_angles;
    this->cloud = cloud;
    this->step_params = step_params;
    this->step = step;
}

float FootTargetFunctor::operator()(float x, float y,
                                    float & av_height, float & av_angle) const
{
    int start_x, end_x, start_y, end_y;
    float height_deviation, angle_deviation;

    /* Вычисление граничных индексов
     *
     * Чтобы вычислить индексы, надо найти смещение границ
     * области наступания относительно границ
     * области поиска. Помним, что x,y - относительно центра
     * области поиска
     */

    int width = cloud->width;
    int height = cloud->height;

    //Если область поиска налезает на
    float search_x = std::min(step_params.SearchX, width*step);
    float search_y = std::min(step_params.SearchY, height*step);

    float left = step_params.SearchY / 2 - y - step_params.FootY/2;     //Расстояние от левой   границы области поиска до левой границы ступни
    float top = step_params.SearchX  / 2 + x - step_params.FootX/2;     //Расстояние от верхней границы области поиска до верхней границы ступни
    float right = left + step_params.FootY;                             //           от левой                          до правой
    float bottom = top + step_params.FootX;                             //           от верхней                        до нижней


    /* В процессе преобразования облака к организованному,
     * оси X соответствует column, а y - row.
     *
     * Но X направлена вперед, Y - влево
     * left, right - ось y
     * top, bottom - ось x
     */

    start_x = top/step;
    end_x   = bottom/step;
    start_y = left/step;
    end_y   = right/step;

    overflow_check(start_x, 0, width,  "start_x");
    overflow_check(end_x,   0, width,  "end_x");
    overflow_check(start_y, 0, height, "start_y");
    overflow_check(end_y,   0, height, "end_y");


    //Расчет параметров
    //TODO: если плоскость под углом ско высот будет большое, но это все равно плоскость!
    av_angle = Math::Average(&angle_func, start_x, end_x, start_y, end_y);
    av_height = Math::Average(&cloud_func, start_x, end_x, start_y, end_y);

    angle_deviation = Math::StandartDeviation(&angle_func, av_angle, start_x, end_x, start_y, end_y);
    height_deviation = Math::StandartDeviation(&cloud_func, av_height, start_x, end_x, start_y, end_y);

    //std::cout<<"M(a): "<<av_angle<<" σ(a): "<<angle_deviation<<"; M(z): "<<av_height<<" σ(z): "<<height_deviation<<std::endl;

    /* Сама целевая функция
     * Лучше когда:
     *  - расстояние от требуемой точки наступания меньше (?)
     *  - СКО нормалей и высот меньше
     *  - Среднее нормали и высот лежат в определенных границах
     */

    //Расстояние текущей точки от центра области поиска
    float dist = sqrt(x*x + y*y);

    /* Построим целевую функцию из нескольких
     * функций "годноты" для каждого параметра:
     *  чем лучше параметр - стремится к 1
     *  чем хуже параметр  - стермится к 0
     *
     *  Для всех параметров наилучшим значение
     *
     *  Для этим целей возьмем функцию Гаусса.
     */

    // Части целевой функции. Параметр - максимальное значение аргумента
    // , при котором значение функции должно стать мало
    // TODO: нормализировать
    double dist_v = log(2*gauss(dist, 0.1));                           //Расстояние от центра
    double av_angle_v = log(2*gauss(av_angle, 30));                    //Средний угол
    double av_height_v = log(2*gauss(av_height, 0.2));                 //Средняя высота
    double angle_deviation_v = log(2*gauss(angle_deviation, 15));      //СКО угла              <- просто шикарно работает
    double height_deviation_v = log(2*gauss(height_deviation, 1));     //СКО высоты            <- непонятно

    //Веса
    double dist_v_w = 0.1;
    double av_angle_v_w = 2;
    double av_height_v_w = 1;
    double angle_deviation_v_w = 1;

    //Хоть я и атеист, но с божьей помощью оно работает
    return  dist_v_w * dist_v +
            angle_deviation_v_w*angle_deviation_v +
            av_angle_v_w * av_angle_v +
            av_height_v_w * av_height_v;
}

void FootTargetFunctor::overflow_check(int value, int min, int max, std::string name) const
{
    if(value < min || value > max)
        throw((name+" is out of range").c_str());
}

//Расчитывает значение функции Гаусса для параметра
double FootTargetFunctor::gauss(float x, float max) const
{
    //Воспользуемся правилом трех сигм
    float sigma = max/3;
    float val =  1/sigma*exp(-x*x/(2*sigma*sigma));

    /* Нормализация
     * при x=0 (max) val = 1/sigma
     * Домножим на sigma
     */
    return sigma*val;


}