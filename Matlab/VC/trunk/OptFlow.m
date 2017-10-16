fig=figure;
set(fig,'DoubleBuffer','on');
aviobj = avifile('hw1.avi', 'FPS', 8);

%step 0
F = double(imread('1bw1.tif')); G = F;
imagesc(F); colormap(gray);
[centerY, centerX] = ginput(1)
centerY = int32(centerY); centerX = int32(centerX);
width = 25;

% Draw white box
disF = F;
disF(centerX - width, centerY-width:centerY+width) = 255;
disF(centerX + width, centerY-width:centerY+width) = 255;
disF(centerX-width:centerX+width, centerY-width) = 255;
disF(centerX-width:centerX+width, centerY+width) = 255;
imagesc(disF); colormap(gray);
aviobj = addframe(aviobj,getframe(fig));

for i = 2 : 8, 
%step 1

% reset varialbles;
F = G;
fname = sprintf('1bw%d.tif',i);
G = double(imread(fname));

roi = zeros(size(F));
roi(centerX - width: centerX + width, centerY - width : centerY + width) = 1;
inds = find(roi); 

sig = 2; % manually set.
x = floor(-3*sig):ceil(3*sig);
GAUSS = exp(-0.5*x.^2/sig^2); 
GAUSS = GAUSS/sum(GAUSS);
dGAUSS = -x.*GAUSS/sig^2;

kernel = GAUSS'*dGAUSS;
Fx = conv2(F,kernel,'same');

kernel = kernel';
Fy = conv2(F,kernel,'same');

Ft = G - F;

C = [sum(Fx(inds).^2)          sum(Fx(inds).*Fy(inds))
     sum(Fx(inds).*Fy(inds))   sum(Fy(inds).^2)];

D = [sum(Ft(inds).*Fx(inds));sum(Ft(inds).*Fy(inds))];

V = pinv(C)*D;

u = V(1); v = V(2);
[rows,cols] = size(F);
[X,Y] = meshgrid(1:cols,1:rows);
Fwarp = interp2(X,Y,F,X+u,Y+v,'linear');

% compute the error ||Ft||^2
Ft = G - Fwarp;
error = sum(Ft(inds).*Ft(inds));
disp(error);

% Draw white box
centerX = centerX - int32(v); centerY = centerY - int32(u);
disF = G;
disF(centerX - width, centerY-width:centerY+width) = 255;
disF(centerX + width, centerY-width:centerY+width) = 255;
disF(centerX-width:centerX+width, centerY-width) = 255;
disF(centerX-width:centerX+width, centerY+width) = 255;
colormap(gray);imagesc(disF);
aviobj = addframe(aviobj,getframe(fig));

end

aviobj = close(aviobj);

