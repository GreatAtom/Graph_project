clear all;


# function [x, N, zmean, zstd, zskewness, zkurtosis] = hstat(z)
function [N, zmean, zstd, zskewness, zkurtosis] = hstat(z)

	# длины, соответствующие частотам
	x = (1:length(z))';

	plot(x,z)

	# общее количество измерений
	N = sum(z);

	# среднее
	zmean = sum(z.*x)/N;

	# при вычитании из отсчёта x(i) - mean(x) частоты не меняются, меняются значения при них

	x_centered = x - zmean;

	#   std (x) = sqrt ( 1/(N-1) SUM_i (x(i) - mean(x))^2 )
	zstd = sqrt(  sum( z.*(x_centered.^2) )/(N-1)  );


	#                          mean ((X - mean (X)).^3)
	#           skewness (X) = ------------------------.
	#                                 std (X).^3
	zskewness = sum( z.*(x_centered.^3) )/N   / zstd^3;

	#                mean ((X - mean (X)).^4)
	#           k1 = ------------------------
	#                       std (X).^4
	zkurtosis =  sum( z.*(x_centered.^4) )/N   / zstd^4  - 3;


endfunction;




function [zmean, zstd, zskewness, zkurtosis] = hstat_norm_0(z)

	# длины, соответствующие частотам
	x = (1:length(z))' - 1;

	plot(x,z)

	# sum(z) =1

	# среднее
	zmean = sum(z.*x);

	# при вычитании из отсчёта x(i) - mean(x) частоты не меняются, меняются значения при них

	x_centered = x - zmean;
	zstd = sqrt(  sum( z.*(x_centered.^2) )  );
	zskewness = sum( z.*(x_centered.^3) )   / zstd^3;
	zkurtosis =  sum( z.*(x_centered.^4) )  / zstd^4  - 3;

endfunction;



#[zmean, zstd, zskewness, zkurtosis] = hstat_norm_0( zmim./sum(zmim) )
#hold on
#[zmean, zstd, zskewness, zkurtosis] = hstat_norm_0( znnd./sum(znnd) )


# [x, N, zmean, zstd, zskewness, zkurtosis] = hstat(z)
# [N, zmean, zstd, zskewness, zkurtosis] = hstat(z)


global column_to_save_length = 25;
# global column_to_save_length = 20;


function [z_column] = f(xz)
	z = xz(:,2);
	[N, zmean, zstd, zskewness, zkurtosis] = hstat(z);
	zn = z/N;
	#  save('bar_al.mat', 'zn')

global column_to_save_length

	z_long = [zn; zeros(column_to_save_length, 1) ];
	z_trimmed = z_long(1:column_to_save_length);

	z_column = [z_trimmed; zmean; zstd; zskewness; zkurtosis];
# 	z_column = [zmean; zstd; zskewness; zkurtosis; z_trimmed];

	#  save('bar_al.mat', 'z_column');

endfunction;

filelist = ls('graph_n*ch.m');
filescount = size(filelist)(1);
for filenum = 1:filescount
# for filenum = 27

	origfilename = strtrim(filelist(filenum,:))

	execfilename = strrep(origfilename, ".m", "");
	eval(execfilename);
	resfilename = strrep(origfilename, ".m", ".mat");

	z_mat = [1:column_to_save_length 0 0 0 0]';
# 	z_mat = [0 0 0 0 1:column_to_save_length]';

	z_mat = [z_mat  f(xz) f(xz0) f(xz1) f(xz2) f(xz3) f(xz4) f(xz5) f(xz6) f(xz7)];



	resfileheader = "x z m0 m1 m2 m3 m4 m5 m6 m7";
	save(resfilename, "resfileheader");
	save("-append", resfilename, "z_mat");

endfor;




