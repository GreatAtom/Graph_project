
Папка src — генерация и анализ графа на C++:
    common.h — настройки (вверху), оценка характеристик распределения (ниже)
    classesgrow_roulette.h — рост различных моделей в виде классов
    classesmain.cpp — головной файл
    
Папка octave — вспомогательные файлы для GNU Octave
    norm_to_stattables.m
    unnorm_to_norm_histstat.m

*******************************************************************************

Сборка/запуск (настройки — в коде, common.h):
g++ -std=c++11 -fopenmp classesmain.cpp && date && ./a.out && date
g++ -std=c++11 -fopenmp classesmain.cpp && date && ./a.out && ./a.out && ./a.out && ./a.out && ./a.out && ./a.out && ./a.out && date && echo 7

количество вершин — NodesCount

алгоритм и параметр роста — virtual const char* title()
ba_98 для Барабаши-Альберт и для ограничения степени вершины

параметр роста — целое число, физический смысл свой для каждого алгоритма роста:
pDel для TNetworkWithDelete
kernelVertices для TCombinedGraphWithBigKernel

время в секундах получается единожды в main() и используется как уникальный идентификатор графа в серии 

Один запуск → выращивание и обсчёт одного графа

Выход:
graph_n<вершин>_<алгоритм>_<параметр>_<число связей у новой вершины>_<максимальное число связей вершины>_id<время>.txt — собственно граф

graph_n<вершин>_<алгоритм>_<параметр>_<число связей у новой вершины>_<максимальное число связей вершины>_id<время>_ch.m — полное РДП xz и его оценки xzi
в формате скрипта GNU Octave
матрица из 2 столбцов: длина пути, количество путей

Моменты и гистограмма помещаются в комментарии и используются только для визуального распознавания файла; в Octave они всё равно потом пересчитываются.

*******************************************************************************

Файлы graph_*_ch.m помещаются в папку с unnorm_to_norm_histstat.
Команда octave unnorm_to_norm_histstat рассчитывает нормированные РДП, их характеристики для всех файлов graph_*_ch.m из текущей папки и помещает файлы graph_*_ch.mat в папку ../norm

Команда octave norm_to_stattables.m рассчитывает по нормированным РДП из папки ./norm разные таблицы и помещает их в папку ./stattables

функция peakdet украдена в
http://www.billauer.co.il/peakdet.html
