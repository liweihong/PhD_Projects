%!src\project.exe -d src\t1.ply

% generate the readable tif for ras2vec.
im = imread('src\\pre1_slice_074.tif');
im = im2bw(im);
imwrite(im, 'ras2vec.tif', 'tif');

%fid = fopen('DP.ply','r');
[rows cols nocare] = size(im);
tt=ones(rows,cols);
figure;imshow(tt);
fid = fopen('DP.ply','r');
X=[]; Y=[];
co = 'r';
i=1;
while 1
    tline = fgetl(fid);
    if ~ischar(tline), break, end
    if (strncmp(tline, 'POLY', 4))
       continue;
    end
	if (strncmp(tline, 'END', 3))
        i = 1;
% 		line(X,Y,'Color', co);
		X=[]; Y=[];
        if (strcmp(co,'r')) 
            co = 'b';
        else co = 'r';
        end
		continue;
	end
	
    A = sscanf(tline, '%d', 2);
    X = [A(1), X];
    %Y = [392-A(2), Y];
    Y = [A(2), Y];
    
    if (i>1)
        X1 = [X(2) X(1)];
        Y1 = [Y(2) Y(1)];
        line(X1,Y1,'Color', co);
        if (strcmp(co,'r')) 
            co = 'b';
        else co = 'r';
        end
    end
    
    i=i+1;
end
fclose(fid);
