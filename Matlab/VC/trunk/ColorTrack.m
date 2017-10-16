%  In matlab you can load a sequence of frames with the aviread function. 
%  For example load the first 20 frames with:  
%  mov = aviread('subject01.avi',[1:20]); 

mov = aviread('motion.avi', [1500]);
%im = mov(1).cdata;
im = circshift(mov(1).cdata, [0,0, -1]);
imshow(im);
[rows,cols,d] = size(im); 
%imshow(im);

% Cancatenate the skin regions.
keepDo = 1;
skin_inds = [];
while (keepDo > 0),
	skin_mask = roipoly;
	skin_inds = [find(skin_mask>0); skin_inds]; 
	size(skin_inds)

	[x y] = ginput(1);
	if (x < 0 | y < 0)
		keepDo = 0;
	end
end

all_R = double(im(:,:,1)); all_G = double(im(:,:,2)); all_B = double(im(:,:,3)); 
skin_data_rgb = [all_R(skin_inds),all_G(skin_inds),all_B(skin_inds)]; % that's the skin data
all_data_rgb = [all_R(:),all_G(:),all_B(:)]; % that's the entire image data 


skin_MN = mean(skin_data_rgb); skin_CV = cov(skin_data_rgb);
%MN = mean(all_data_rgb);
%CV = cov(all_data_rgb);
P_skin = gaussdensity(all_data_rgb, skin_MN, skin_CV);
L1 = reshape(P_skin,rows,cols); 

% Cancatenate the background regions.
disp('Choose background region');
keepDo = 1;
bg_inds = [];
while (keepDo > 0),
	bg_mask = roipoly;
	bg_inds = [find(bg_mask>0); bg_inds]; 
	size(bg_inds)

	[x y] = ginput(1);
	if (x < 0 | y < 0)
		keepDo = 0;
	end
end

disp('Start working on background region');
bg_data_rgb = [all_R(bg_inds),all_G(bg_inds),all_B(bg_inds)]; % that's the bg data


bg_MN = mean(bg_data_rgb); bg_CV = cov(bg_data_rgb);
P_bg = gaussdensity(all_data_rgb, bg_MN, bg_CV);
L2 = reshape(P_bg, rows, cols);

binIm = zeros(rows,cols);
skinReg = find (L1>L2);
binIm(skinReg) = 1;
imshow(binIm);



