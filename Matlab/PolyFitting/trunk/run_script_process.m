% File: run_script_process.m
% 
% This file is used to process the generated image file to get line fittings .
%
% Algorithm: Run the commands
%
data_dir = 'data/img';
out_dir = 'data/processed_post_noise_psu_inside';
for i = 40:55
    % add the leading 0s.
    if i < 10, str_pad = '00'; 
    elseif i < 100, str_pad = '0';
    else str_pad = '';
    end
    
    filename=sprintf('%s/Rotated_split_%s%d.tiff',data_dir, str_pad, i);
    image = imread(filename);
    % with data recover and preprocess 1
    image = double(data_recover(preprocess(image)));
    %image = double(preprocess(image));  
    
    % with preprocess 2
    % image = double(preprocess_2(image));  
    % image = preprocess_2(image);
    
    out_filename = sprintf('%s/Done_Rotated_split_%s%d.tiff',out_dir, str_pad, i);
    imwrite(image, out_filename, 'TIFF');
   	disp(sprintf('%d finished', i));
%    disp(out_filename);
end

