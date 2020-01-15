clear all;



# *****************************************************************************
# Статистика по моменту midx / оценкам момента midx по выборкам:
# — значение момента midx (по всему графу)
# —  min, среднее, max, СКО по оценкам
# группа для подсчёта статистики — один граф
# — ошибки среднего оценок
function [moment_line] = mm(z_moments, midx)
# 	[0  g(xz) g(xz0) g(xz1) g(xz2) g(xz3) g(xz4) g(xz5) g(xz6) g(xz7)];	
	z_moment = z_moments(midx, :);
	mfull = z_moment(2);
	mparts = z_moment(3:10);
	
	dm = mparts - mfull;
	
# 	moment_line = [mfull min(mparts) quantile(mparts,0.25) median(mparts)  mean(mparts) quantile(mparts,0.75) max(mparts)  std(mparts)  mean(dm) mean(dm)/mfull*100];
	moment_line = [mfull min(mparts) quantile(mparts,0.25) median(mparts)  mean(mparts) quantile(mparts,0.75) max(mparts)  std(mparts)  mean(dm) mean(dm)/abs(mfull)*100];

endfunction;

# # *****************************************************************************
# # Одно значение момента midx (по всему графу)
# function [mfull] = momentfull(z_moments, midx)
# # 	[0  g(xz) g(xz0) g(xz1) g(xz2) g(xz3) g(xz4) g(xz5) g(xz6) g(xz7)];	
# 	z_moment = z_moments(midx, :);
# 	mfull = z_moment(2);
# endfunction;

# *****************************************************************************
# Сортировка так, чтобы 1000 был раньше 100000
function [filelist, vxSorted] = sort_file_list(filelist_raw, X_ba)
	filescount = size(filelist_raw)(1);

# 	filelist_to_sort = [];
# 	for filenum = 1:filescount
# 		f = strrep(filelist_raw(filenum,:), "_", "-");
# 		filelist_to_sort = [filelist_to_sort; f];
# 	endfor;
# 
# 	filelist_sorted = sortrows(filelist_to_sort);
# 
# 	filelist = [];
# 	for filenum = 1:filescount
# 		f = strrep(filelist_sorted(filenum,:), "-", "_");
# 		filelist = [filelist; f];
# 	endfor;


	filelist_to_sort = [];
	for filenum = 1:filescount
		origfilename = strtrim(filelist_raw(filenum,:));		
		[nvertices X] = extract_v_key(origfilename, X_ba);
		filelist_to_sort = [filelist_to_sort; nvertices X];
	endfor;

	[vxSorted, I] = sortrows(filelist_to_sort, [2 1]);
    filelist = filelist_raw(I,:)

endfunction;


# # *****************************************************************************
# # Статистика по ОДНОЙ величине: кол-во, min, среднее, max, СКО
# # data — трёхколоночная матрица		data = [v X z1];
# # группы для подсчёта статистики — по v и X
# function [moment_mat] = single_stat_by_vX_group(data)
# 	vX = unique(data(:,1:2), "rows");
# 	vXlen =  size(vX)(1);
# 	
# 	moment_mat = [];
# 	for i = 1:vXlen
# 		v = vX(i,1);
# 		X = vX(i,2);
# 		idx = (data(:,1) == v) & (data(:,2) == X);
# 		z = data(idx,3);
# 		idcount = size(z)(1);
# 		moment_mat = [moment_mat; v X idcount min(z) mean(z) max(z)  std(z) ];
# 	endfor;
# 	
# 	moment_mat = sortrows(moment_mat, 2);
# endfunction;


# # *****************************************************************************
# # Среднее по КАЖДОМУ столбцу data, кроме ключей v и X
# # data — многоколоночная матрица 	data = [v X z1 z2 ...];	
# # группы для подсчёта статистики — по v и X
# # итоговая сортировка — по X
# function [moment_mat] = matrix_stat_by_vX_group(data)
# 	vm = unique(data(:,1:2), "rows");
# 	vmlen =  size(vm)(1);
# 	
# 	strlen =  size(data)(2);
# 	
# 	moment_mat = [];
# 	for i = 1:vmlen
# 		v = vm(i,1);
# 		X = vm(i,2);
# 		idx = (data(:,1) == v) & (data(:,2) == X);
# 		vxz = data(idx,:);
# 		#moment_mat = [moment_mat;  mean(vxz, 1)];
# 		matrix_std_line = std(vxz, 0, 1); # два нуля (СКО v и X) и СКО моментов
# 		moment_mat = [moment_mat;  mean(vxz, 1) matrix_std_line(3:strlen)];
# 	endfor;
# 	
# 	moment_mat = sortrows(moment_mat, 2);
# endfunction;



# *****************************************************************************
# регистр различается
function key = inf_key; key = 2147483647; endfunction;
function key = field_m_key; key = -1357; endfunction;
function key = field_M_key; key = -2468; endfunction;
# *****************************************************************************

# *****************************************************************************
# Поля из имени файла: v и X
# v — количество вершин, |G|, первое поле в имени файла filename
# X — значение после буквенного обозначения модели в имени файла filename
# X_ba — то, что надо подставлять вместо X для модели Барабаши-Альберт (там нет значения X)
function [nvertices X] = extract_v_key(filename, X_ba)
	fields = sscanf(filename, "norm/graph_n%d_%[^_]_%d_%d_%d_id%d_ch.mat");
	nvertices = fields(1);
 	# строка-метка в fields посимвольно!!! так что все поля, начиная с 2, имеют плавающий номер
	nfields =  size(fields)(1);
	X = fields(nfields-3);
	
 	if (98 == X) # хвост от 'ba', но почему-то не ascii-код 'a'. А теперь сигнатура
		if (field_m_key == X_ba)
			X = fields(nfields-2); # m
		elseif (field_M_key == X_ba)
			X = fields(nfields-1); # M
		else
			X = X_ba;
		endif;
	endif;
endfunction;

# *****************************************************************************
# Названия полей в заголовок файла вместо X
# X_ba — то, что надо подставлять вместо X для Барабаши-Альберт
function [fileheader] = file_header_by_key(fileXheader, X_ba)
	if (field_m_key == X_ba)
		fileheader = strrep(fileXheader, "X", "m");
	elseif (field_M_key == X_ba)
		fileheader = strrep(fileXheader, "X", "M");
	else
		fileheader = fileXheader;
	endif;
endfunction;


# *****************************************************************************
# # Извлечение из *_ch.mat по списку только полных моментов
# # и сохранение одной сводной таблицы по всем моментам
# #
# # do_matrix_stat_by_vX_group = 1 — средние по группам, группа = по v и X
# # 
# # do_matrix_stat_by_vX_group = 0 — строк столько же, сколько графов
# # 
# # X_ba — то, что надо подставлять вместо X (значение после буквенного обозначения модели в имени файла) для Барабаши-Альберт
# function  save_moments_full_combine(unsortfilelist, filenamepattern, do_matrix_stat_by_vX_group, X_ba)
# 	
# 	data = [];
# 
# 	#filelist = sort_file_list(  unsortfilelist  );
# 	[filelist] = sort_file_list(unsortfilelist, X_ba)
# 	filescount = size(filelist)(1);
# 	for filenum = 1:filescount
# 		origfilename = strtrim(filelist(filenum,:));
# 		[nvertices X] = extract_v_key(origfilename, X_ba);
# 		
# 		load(origfilename, "combine_mat");
# 		data = [data; nvertices X momentfull(combine_mat, 101) momentfull(combine_mat, 102) momentfull(combine_mat, 103) momentfull(combine_mat, 104) momentfull(combine_mat, 105)];	
# 		
# 	endfor;
# 
# 	# fileheader = file_header_by_key("v X mode mean std as ex", X_ba);
# 	fileheader = file_header_by_key("v X mode mean std as ex modestd meanstd stdstd asstd exstd", X_ba);
# 
# 	data = sortrows(data, 2);
# 
# 	if (do_matrix_stat_by_vX_group > 0)
# 		data = matrix_stat_by_vX_group(data);
# 	endif;
# 
# 	filename = sprintf(filenamepattern, "moments");
# 	save(filename, "fileheader");
# 	save("-append", filename, "data");
# endfunction;


# # *****************************************************************************
# # Извлечение из *_ch.mat по списку только полных моментов
# # и сохранение раздельных сводных таблиц по моментам (средние, разброс)
# # группа = по v и X
# # X_ba — то, что надо подставлять вместо X (значение после буквенного обозначения модели в имени файла) для Барабаши-Альберт
# function  save_moments_full_split_groupstat(unsortfilelist, filenamepattern, X_ba)
# 	meandata = [];
# 	stddata = [];
# 	asdata = [];
# 	exdata = [];
# 
# 	#filelist = sort_file_list(  unsortfilelist  );
# 	[filelist] = sort_file_list(unsortfilelist, X_ba)
# 	filescount = size(filelist)(1);
# 	for filenum = 1:filescount
# 		origfilename = strtrim(filelist(filenum,:));
# 		
# 		[nvertices X] = extract_v_key(origfilename, X_ba);
# 		
# 		load(origfilename, "combine_mat");
# 		meandata = [meandata; nvertices X momentfull(combine_mat, 102)];	# 27 — среднее
# 		stddata  = [stddata;  nvertices X momentfull(combine_mat, 103)];	# 28 — СКО
# 		asdata = [asdata; nvertices X momentfull(combine_mat, 104)];	# 29 — асимметрия
# 		exdata = [exdata; nvertices X momentfull(combine_mat, 105)];	# 30 — эксцесс
# 	endfor;
# 	meandata = single_stat_by_vX_group(meandata);
# 	stddata = single_stat_by_vX_group(stddata);
# 	asdata = single_stat_by_vX_group(asdata);
# 	exdata = single_stat_by_vX_group(exdata);
# 	
# 	fileheader = file_header_by_key("v X count min mean max std", X_ba);
# 
# 	filename = sprintf(filenamepattern, "mean");
# 	save(filename, "fileheader");
# 	save("-append", filename, "meandata");
# 
# 	filename = sprintf(filenamepattern, "std");
# 	save(filename, "fileheader");
# 	save("-append", filename, "stddata");
# 
# 	filename = sprintf(filenamepattern, "asimm");
# 	save(filename, "fileheader");
# 	save("-append", filename, "asdata");
# 
# 	filename = sprintf(filenamepattern, "ex");
# 	save(filename, "fileheader");
# 	save("-append", filename, "exdata");
# endfunction;

# *****************************************************************************
# регистр различается
function key = idposdelta; key = 0.5; endfunction;
# function key = idposdelta; key = 1; endfunction;
# function idpos = idpos_by_v(idcounter, nvertices); 
#   idpos = idcounter-1 + (2*idposdelta + 1)*(log10(nvertices)-2) - idposdelta; 
function idpos = idpos_by_v(idcounter, idgroup); 
   idpos = idcounter-1 + (2*idposdelta + 1)*(idgroup-1) + idposdelta+1; 
endfunction;
# *****************************************************************************
# Среднее по КАЖДОМУ столбцу data, кроме ключей v и X, и~idpos
# data — многоколоночная матрица 	data = [v X idpos z1 z2 ...];	
# группы для подсчёта статистики — по v и X
# итоговая сортировка — по X
# v X idpos mean imean std fq tq
function [moment_mat] = posfullpart_stat_by_vX_group(data)
	vX = unique(data(:,1:2), "rows");
	vXlen =  size(vX)(1);
	
	strlen =  size(data)(2); # количество столбцов в data, включая v, X, idpos 
	
	moment_mat = [];
	for i = 1:vXlen
		v = vX(i,1); # количество вершин |G|
		X = vX(i,2); # параметр моделирования (M/p/K)
		idx = (data(:,1) == v) & (data(:,2) == X);
		vxz = data(idx,:); # часть матрицы [v X idpos z1 z2 ...] с постоянными v и X
		zcount=size(vxz)(1);# количество строк
		idpos = max(vxz(:,3)) + idposdelta + 1; # max(idpos элементов в группе) + одна граница + 1
		# все строки, включая выбросы, но без столбцов v, X, idpos
		z = vxz(:, 4:strlen);
		meanz = mean(z, 1); # среднее по всем элементам (обычное) по столбцам (1)
		stdz  = std(z, 0, 1); # СКО по всем элементам (обычное) с~делением на n-1 (0) по столбцам (1)
		fqz   = quantile(z, 0.25, 1); # Q1 по всем элементам по столбцам (1)
		tqz   = quantile(z, 0.75, 1); # Q3 по всем элементам по столбцам (1)
		# удаление выбросов
		medianz = median(z, 1);
		# границы внутреннего интервала
		# idelta = 3*stdz;
		# idelta = 1.5*stdz;
		idelta = 2*stdz;
		imin = medianz - idelta;
		imax = medianz + idelta;
		# фильтры по столбцам
        imeanz = [];
        zcols =  size(z)(2); # количество столбцов в z (только данные, без v, X, idpos)
		for j = 1:zcols
            jz = z(:,j); 
            lo = imin(:,j); 
            hi = imax(:,j); 
            inneridx = (jz >= lo) & (jz <= hi); 
            innerz = jz(inneridx);
            # для моды бывает std = 0
            imeanz = [imeanz mean(innerz)]; 
        endfor;
		minz = min(z, [], 1);
		maxz = max(z, [], 1);
		# итоговая строка статистики
        # левые столбцы в начало не добавлять, при > 32 графопростраиваемых столбцах будет Could not parse input as a floating point number
        # а в таблицах вроде можно
		# moment_mat = [moment_mat; v X idpos meanz imeanz stdz minz fqz tqz maxz     zcount];
		moment_mat = [moment_mat; v X idpos meanz  stdz minz fqz tqz maxz     zcount];
	endfor;
# 	moment_mat
# 	vX
# 	data
	
	# здесь сортировка нужна, потому что, хотя data в правильном порядке, vX сортирован по v
	moment_mat = sortrows(moment_mat, 2); # по X
	
	totalmeanline = mean(moment_mat, 1);
	# totalmeanline(1,1) = 1; # v
	# totalmeanline(1,2) = -1; # X
	totalmeanline(1,3) = 0; # idpos
		
    moment_mat = [totalmeanline; moment_mat];
	
endfunction;
# *****************************************************************************
# Извлечение из *_ch.mat по списку всех моментов, полных и частичных
# и сохранение раздельных сводных таблиц по моментам (полный, средние частичных, разброс частичных, ошибка частичных)
# группа = граф
# X_ba — то, что надо подставлять вместо X (значение после буквенного обозначения модели в имени файла) для Барабаши-Альберт
function  save_moments_full_and_part(unsortfilelist, filenamepattern, X_ba)
	modedata = [];
	meandata = [];
	stddata = [];
	asdata = [];
	exdata = [];
	
	vXgroups = [];
	groupindex = 0;
	id_in_group = 0;

	#filelist = sort_file_list(  unsortfilelist  );
	[filelist] = sort_file_list(unsortfilelist, X_ba)
	filescount = size(filelist)(1);
	for filenum = 1:filescount
		origfilename = strtrim(filelist(filenum,:));		
		[nvertices X] = extract_v_key(origfilename, X_ba);
		if ( isempty(find(ismember(vXgroups,[[nvertices X]],'rows')))  )
            vXgroups = [vXgroups; nvertices X];
            groupindex = groupindex+1;
            printf("%d ", groupindex)
            id_in_group = 0;
		endif
		id_in_group = id_in_group + 1;

	# vXgroups = unique([nvertices X], "rows"); пока это одна пара
    
    # 	idpos = filenum + 3*(log10(nvertices)) - 8;
    # idpos = idpos_by_v(filenum, nvertices); 
    idpos = idpos_by_v(filenum, groupindex); 
		
		load(origfilename, "combine_mat");
	# 	[i  f(xz) f(xz0) f(xz1) f(xz2) f(xz3) f(xz4) f(xz5) f(xz6) f(xz7)]; 
	# 	[0  g(xz) g(xz0) g(xz1) g(xz2) g(xz3) g(xz4) g(xz5) g(xz6) g(xz7)];	
		
# 		modedata = [modedata; filenum nvertices X idpos mm(combine_mat, 26)];	# 26 — мода
# 		meandata = [meandata; filenum nvertices X idpos mm(combine_mat, 27)];	# 27 — среднее
# 		stddata  = [stddata;  filenum nvertices X idpos mm(combine_mat, 28)];	# 28 — СКО
# 		asdata = [asdata; filenum nvertices X idpos mm(combine_mat, 29)];	# 29 — асимметрия
# 		exdata = [exdata; filenum nvertices X idpos mm(combine_mat, 30)];	# 30 — эксцесс

        zcol = combine_mat(:,2); # первый столбец — нумерация, второй — полное РДП
        z = zcol(1:100);
        [maxtab, mintab]=peakdet(z, 0.002);
        peaksnum = size(maxtab)(1);
		
# 		modedata = [modedata; filenum nvertices X idpos peaksnum mm(combine_mat, 101)];	# 101 — мода
# 		meandata = [meandata; filenum nvertices X idpos peaksnum mm(combine_mat, 102)];	# 102 — среднее
# 		stddata  = [stddata;  filenum nvertices X idpos peaksnum mm(combine_mat, 103)];	# 103 — СКО
# 		asdata = [asdata; filenum nvertices X idpos peaksnum mm(combine_mat, 104)];	# 104 — асимметрия
# 		exdata = [exdata; filenum nvertices X idpos peaksnum mm(combine_mat, 105)];	# 105 — эксцесс

        common_begin = [filenum nvertices X idpos];
        common_end   = [peaksnum id_in_group]; # в начало не добавлять, чтобы не сдвигались vxstat_columns
    
		
		modedata = [modedata; common_begin mm(combine_mat, 101) common_end];	# 101 — мода
		meandata = [meandata; common_begin mm(combine_mat, 102) common_end];	# 102 — среднее
		stddata  = [stddata;  common_begin mm(combine_mat, 103) common_end];	# 103 — СКО
		asdata = [asdata; common_begin mm(combine_mat, 104) common_end];	# 104 — асимметрия
		exdata = [exdata; common_begin mm(combine_mat, 105) common_end];	# 105 — эксцесс
	endfor;
# 	modedata = sortrows(modedata, 2);
# 	meandata = sortrows(meandata, 2);
# 	stddata = sortrows(stddata, 2);
# 	asdata = sortrows(asdata, 2);
# 	exdata = sortrows(exdata, 2);
# 	
# 	modedata = sortrows(modedata, 3);
# 	meandata = sortrows(meandata, 3);
# 	stddata = sortrows(stddata, 3);
# 	asdata = sortrows(asdata, 3);
# 	exdata = sortrows(exdata, 3);
# 	
	fileheader = file_header_by_key("idcounter v X idpos  full min fq median mean tq max std abserr relerr peaksnum r", X_ba);

	filename = sprintf(filenamepattern, "mode");
	save(filename, "fileheader");
	save("-append", filename, "modedata");

	filename = sprintf(filenamepattern, "mean");
	save(filename, "fileheader");
	save("-append", filename, "meandata");

	filename = sprintf(filenamepattern, "std");
	save(filename, "fileheader");
	save("-append", filename, "stddata");

	filename = sprintf(filenamepattern, "asimm");
	save(filename, "fileheader");
	save("-append", filename, "asdata");

	filename = sprintf(filenamepattern, "ex");
	save(filename, "fileheader");
	save("-append", filename, "exdata");
	
    # "idcounter v X idpos full min fq median mean tq max std abserr relerr  ",
    #   1        2 3 4     5    6   7  8      9    10 11  12  13     14
    vxstat_columns = [2:4 5 9 13 14];
	# v X idpos  [ mean imean std fq tq ] x [ full partavg abserr relerr ]
	# v X idpos  meanfull meanpartavg meanabserr meanrelerr  imeanfull imeanpartavg imeanabserr imeanrelerr  stdfull stdpartavg stdabserr stdrelerr  fqfull fqpartavg fqabserr fqrelerr  tqfull tqpartavg tqabserr rtqelerr 
	# fileheader = file_header_by_key("v X idpos  meanfull meanpartavg meanabserr meanrelerr  imeanfull imeanpartavg imeanabserr imeanrelerr  stdfull stdpartavg stdabserr stdrelerr  fqfull fqpartavg fqabserr fqrelerr  tqfull tqpartavg tqabserr rtqelerr", X_ba);
# 	fileheader = "v X idpos  meanfull meanpartavg meanabserr meanrelerr  imeanfull imeanpartavg imeanabserr imeanrelerr  stdfull stdpartavg stdabserr stdrelerr  fqfull fqpartavg fqabserr fqrelerr  tqfull tqpartavg tqabserr tqrelerr"; # не меняем X на m/M, пусть будет единообразие

#     # v X count idpos  [ mean imean std min fq tq max ] x [ full partavg abserr relerr ]
# 	fileheader = "v X idpos  meanfull meanpartavg meanabserr meanrelerr  imeanfull imeanpartavg imeanabserr imeanrelerr  stdfull stdpartavg stdabserr stdrelerr  minfull minpartavg minabserr minrelerr fqfull fqpartavg fqabserr fqrelerr  tqfull tqpartavg tqabserr tqrelerr  maxfull maxpartavg maxabserr maxrelerr   count "; # не меняем X на m/M, пусть будет единообразие
    # v X count idpos  [ mean  std min fq tq max ] x [ full partavg abserr relerr ]
	fileheader = "v X idpos  meanfull meanpartavg meanabserr meanrelerr  stdfull stdpartavg stdabserr stdrelerr  minfull minpartavg minabserr minrelerr fqfull fqpartavg fqabserr fqrelerr  tqfull tqpartavg tqabserr tqrelerr  maxfull maxpartavg maxabserr maxrelerr   count "; # не меняем X на m/M, пусть будет единообразие

	
	
	modegroup = posfullpart_stat_by_vX_group(modedata(:,vxstat_columns));
	meangroup = posfullpart_stat_by_vX_group(meandata(:,vxstat_columns));
	stdgroup = posfullpart_stat_by_vX_group(stddata(:,vxstat_columns));
	asgroup = posfullpart_stat_by_vX_group(asdata(:,vxstat_columns));
	exgroup = posfullpart_stat_by_vX_group(exdata(:,vxstat_columns));

	filenamepattern = strrep(filenamepattern, "_by_id", "_by_vx");
	filename = sprintf(filenamepattern, "mode");
	save(filename, "fileheader");
	save("-append", filename, "modegroup");

	filename = sprintf(filenamepattern, "mean");
	save(filename, "fileheader");
	save("-append", filename, "meangroup");

	filename = sprintf(filenamepattern, "std");
	save(filename, "fileheader");
	save("-append", filename, "stdgroup");

	filename = sprintf(filenamepattern, "asimm");
	save(filename, "fileheader");
	save("-append", filename, "asgroup");

	filename = sprintf(filenamepattern, "ex");
	save(filename, "fileheader");
	save("-append", filename, "exgroup");	
endfunction;



# *****************************************************************************

# Извлечение из *_ch.mat по списку только полных моментов
# и сохранение одной сводной таблицы по всем моментам

# с усреднением по группам  по v и X
# save_moments_full_combine(ls('norm/graph_n10000_ba_98_1_*_ch.mat'), "stattables/bamax_tree_%s_by_vm.mat", 1, field_M_key) # одна таблица по всем моментам



# save_moments_full_combine([ls('norm/graph_n10000_del_*_ch.mat'); ls('norm/graph_n10000_ba_98_1_214*_ch.mat')], "stattables/del_tree_%s_by_vx.mat", 1, 0)
# save_moments_full_combine([ls('norm/graph_n1000*_pfk_*_ch.mat'); ls('norm/graph_n1000*_ba_98_1_214*_ch.mat')], "stattables/pfk_tree_%s_by_vx.mat", 1, inf_key)

# без усреднения, но в каждой строке — все моменты
# save_moments_full_combine(ls('graph_n10000_del_*_ch.mat'), "del_tree_%s_by_id.mat", 0)

# *****************************************************************************

# Извлечение из *_ch.mat по списку только полных моментов
# и сохранение раздельных сводных таблиц по моментам (средние, разброс)
# группа = по v и X

# save_moments_full_split_groupstat(ls('norm/graph_n*_ba_98_*_214*_ch.mat'), "stattables/ba_%s_by_vm.mat", field_m_key)

# *****************************************************************************

# Извлечение из *_ch.mat по списку всех моментов, полных и частичных
# и сохранение раздельных сводных таблиц по моментам (полный, средние частичных, разброс частичных, ошибка частичных)
# группа = граф

save_moments_full_and_part(ls('norm/graph_n*_ba_98_*_214*_ch.mat'), "stattables/ba_%s_by_id.mat", field_m_key) # не только деревья

# save_moments_full_and_part(ls('norm/graph_n*_ba_98_1_214*_ch.mat'), "stattables/ba_tree_%s_by_id.mat", field_M_key)
# save_moments_full_and_part(ls('graph_n1000*_pfk_*_ch.mat'), "pfk_tree_%s_by_id.mat", inf_key)
#save_moments_full_and_part(ls('graph_n1000*_pfk_*_ch.mat'), "pfk_tree_%s_by_id.mat", inf_key)
# save_moments_full_and_part(ls('norm/graph_n100000_pfk_*_ch.mat'), "stattables/pfk_tree_5_%s_by_id.mat", inf_key)

save_moments_full_and_part(ls('norm/graph_n*_ba_98_1_214*_ch.mat'), "stattables/ba_tree_%s_by_id.mat", field_M_key)

save_moments_full_and_part(ls('norm/graph_n10000_ba_98_1_*_ch.mat'), "stattables/bamax_byX_tree_%s_by_id.mat", field_M_key) # создаётся много таблиц


save_moments_full_and_part([ls('norm/graph_n10000_del_*_ch.mat'); ls('norm/graph_n10000_ba_98_1_214*_ch.mat')], "stattables/del_byX_tree_%s_by_id.mat", 0)


save_moments_full_and_part(ls('norm/graph_n*_pfk_16_1_*_ch.mat'), "stattables/pfk_16_tree_%s_by_id.mat", inf_key)
save_moments_full_and_part(ls('norm/graph_n*_pfk_32_1_*_ch.mat'), "stattables/pfk_32_tree_%s_by_id.mat", inf_key)
save_moments_full_and_part(ls('norm/graph_n*_pfk_256_1_*_ch.mat'), "stattables/pfk_256_tree_%s_by_id.mat", inf_key)

save_moments_full_and_part([ls('norm/graph_n1000*_pfk_*_ch.mat'); ls('norm/graph_n1000*_ba_98_1_214*_ch.mat')], "stattables/pfk_all_tree_%s_by_id.mat", inf_key)


# *****************************************************************************




