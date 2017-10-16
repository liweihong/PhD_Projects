function P = gaussdensity(all_data_rgb,MN,CV)

% Nx3 matrix - 1x3 vector
all_data_rgb(:,1) = all_data_rgb(:,1) - MN(1);
all_data_rgb(:,2) = all_data_rgb(:,2) - MN(2);
all_data_rgb(:,3) = all_data_rgb(:,3) - MN(3);

[d, col] = size(CV);

% Gaussian multivarite equation with only matrix operation.
P = (exp(-0.5*all_data_rgb*inv(CV).*all_data_rgb))/((2*pi)^(d/2)*det(CV)^0.5);
P = sum(P,2); % like the matrix multiplication.

end
