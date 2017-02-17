/**
 * Параметры расчета шага
 *
 * Эта структура хранит все параметры, которые требуются для расчета шагов.
 * Имеются в виду более-менее постоянные параметры (настройки), которые
 * не меняются каждый шаг
 *
 */

#ifndef AR600_VISION_STEPSPARAMS_H
#define AR600_VISION_STEPSPARAMS_H

namespace StepsController
{

    struct StepsParams
    {
        //Параметры преобразования облака
        float DownsampleLeafSize;                   // Параметр для уменьшения количества точек
        float ShiftX, ShiftY, ShiftZ;               // Сдвиг облака точек, чтобы привести начало С.К.
                                                    //    облака к началу С.К. робота
        float RotX, RotY, RotZ;                     // Поворот облака, чтобы компенсировать наклон облака
                                                    //    (помимо константных поворотов для соответствия осей)

        //Параметры обработки
        float NormalSearchRadius;                   // Радиус поиска нормалей

        //Параметры поиска места наступания
        float FootX, FootY;                         // Размеры ступни
        float SearchX, SearchY, SearchZ;            // Размеры области поиска
    };

}

#endif //AR600_VISION_STEPSPARAMS_H
