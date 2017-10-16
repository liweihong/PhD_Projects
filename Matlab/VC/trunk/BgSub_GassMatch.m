function [P, Q] = BgSub_GassMatch(im,mu,sigma)

all_R = double(im(:,:,1)); all_G = double(im(:,:,2)); all_B = double(im(:,:,3)); 
all_data = [all_R(:),all_G(:),all_B(:)]; % that's the entire image data 
all_R = double(mu(:,:,1)); all_G = double(mu(:,:,2)); all_B = double(mu(:,:,3)); 
all_mu = [all_R(:),all_G(:),all_B(:)]; % that's the mean image
all_sigma = sigma(:); 			% vector

all_data = all_data - all_mu; % difference matrix
P = all_data.*all_data; 
P = sum(P,2); 					% squred and summation
Q = P;
P = P.*(1./(all_sigma));  % get the final equation.
