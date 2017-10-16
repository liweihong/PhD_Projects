% File: hunter_load_data.m
%
% This is Matlab code for data loading. 
% Click <Matlab_hunter_load_data.m> for the source code.
% 
% Here is the file format:                                                  
%                                                                           
%  Each file contains a list of points.                                     
%  Each point is represented with nine parameters:                          
%                                                                           
%   x y z n1 n2 n3 r a b                                                    
%                                                                           
%  (x,y,z):       3D point                                                  
%  (n1,n2,n3): normal                                                       
%  r: reflectance                                                           
%  a: quality of normal (the smaller the number the better the normal       
% calculation)                                                              
%  b: scan number                                                           
%                                                                           
%  For the purposes of this project only the 3D coordinates of the point    
% (maybe its normal as well) need to be used.                               
% 
%
% Pre-Condition:
%
% char* filename
%
% Post-Condition:
% 
% Tiff image extracted from the cloud points data. 
%
% Implementation:
%
% - Read the data files twice: first one is to get the line number to construct the array;
% second time is to load the data into the array. 
% - Two parameters for image generation. First one is the scale facotr (5), which is used to multiply
% the coordinates. The second one is image size (540x360), which should be appropriate value according to
% the maximum value of the scaled coordinates.

% Function: hunter_load_data
% 
% This is Matlab code for data loading.
% 
function [image] = hunter_load_data (filename)
% Read the point cloudy
%filename = 'Rotated_split_037.bpa';
fid = fopen(filename, 'r');
%Get the lines of data
i = 0;
while 1
    tline = fgetl(fid);
    if ~ischar(tline), break, end
    i = i + 1;
end
B = zeros(i,2);
frewind(fid);

i = 1;
while 1
    tline = fgetl(fid);
    if ~ischar(tline), break, end
    A = sscanf(tline, '%f', 3);
    B(i, 1) = abs(A(1));
    B(i, 2) = abs(A(3));
    i= i + 1;
end
fclose(fid);

%reshape B
D = uint32(5*B);
max(D)
imSize1 = 360; imSize2 = 540;
C = ones(imSize1,imSize2);
% assign the coords to image.
Index = (D(:,2) - 1) * imSize1 + D(:,1);
% find the set of the Index
Index = unique(Index, 'rows');
% make sure the boundary of the index.
IndexIn = find(Index<imSize1*imSize2);
C(Index(IndexIn)) = 0;
C = logical(C);
% rotate the image if necessary.
C = imrotate(C, 180);

% imwrite(C, sprintf('bitwise_%s_img.tiff',filename), 'TIFF');
% return the image
image = C;