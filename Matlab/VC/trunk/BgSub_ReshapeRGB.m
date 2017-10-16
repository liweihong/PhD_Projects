function P = BgSub_ReshapeRGB(im)

all_R = double(im(:,:,1)); all_G = double(im(:,:,2)); all_B = double(im(:,:,3)); 
P = [all_R(:),all_G(:),all_B(:)]; % that's the entire image data 

