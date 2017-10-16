% File: run_script.m
% 
% This file is used to run the command as a script.
%
% First: load the data and convert them into images
%
% Second: run the commands on the images.

data_dir = 'data/slices';
out_dir = 'data/img';
for i = 0:100
    % add the leading 0s.
    if i < 10, str_pad = '00'; 
    elseif i < 100, str_pad = '0';
    else str_pad = '';
    end
    filename=sprintf('%s/Rotated_split_%s%d.bpa',data_dir, str_pad, i);
    image = hunter_load_data(filename);
    out_filename = sprintf('%s/Rotated_split_%s%d.tiff',out_dir, str_pad, i);
    imwrite(image, out_filename, 'TIFF');
%    disp(filename);
%    disp(out_filename);
end