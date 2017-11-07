function out = get_feature_map(im_patch, features, w2c)

% out = get_feature_map(im_patch, features, w2c)
%
% Extracts the given features from the image patch. w2c is the
% Color Names matrix, if used.

if nargin < 3
    w2c = [];
end

% the names of the features that can be used
valid_features = {'gray', 'cn'};    %可用的特征

% the dimension of the valid features
feature_levels = [1 10]';         %

num_valid_features = length(valid_features);   %2
used_features = false(num_valid_features, 1);  % 2*1的矩阵全0

% get the used features
for i = 1:num_valid_features    %2
    used_features(i) = any(strcmpi(valid_features{i}, features));   %看这里一样不，只有一样的时候才会把use_feature这个置为1
end
% used_features  [1,0]         [0,1]         [1,  1]
%                gray no       no cn         gray  cn
% total number of used feature levels
num_feature_levels = sum(feature_levels .* used_features);
%注意是点乘，用灰度的话是1，用cn的话是10

level = 0;

% If grayscale image
if size(im_patch, 3) == 1      %如果是灰度图像的话就只能用灰度特征了
    % Features that are available for grayscale sequances
    
    % Grayscale values (image intensity)
    out = single(im_patch)/255 - 0.5;    %归一化去中心
else 
    % Features that are available for color sequances
    
    % allocate space (for speed)   先分配空间，
    out = zeros(size(im_patch, 1), size(im_patch, 2), num_feature_levels, 'single');    %看给多少通道，single占四个字节，算起来稍快，四个字节也完全够用
    
    % Grayscale values (image intensity)
    if used_features(1)        %如果用灰度的话
        out(:,:,level+1) = single(rgb2gray(im_patch))/255 - 0.5;    %灰度话->归一化->中心化，给第一通道，如果用gray的也就只有一个通道了
        level = level + feature_levels(1);       %这时level=0+1为1
    end
    
    % Color Names       如果是用颜色空间的话
    if used_features(2)
        if isempty(w2c)   %如果没有加载到w2c
            % load the RGB to color name matrix if not in input
            temp = load('w2crs');
            w2c = temp.w2crs;
        end
        
        % extract color descriptor
        out(:,:,level+(1:10)) = im2c(single(im_patch), w2c, -2);   %这里就是另外一个函数了,得到十个通道的特征，这里的细节还没有搞懂，再看
        level = level + feature_levels(2);      %看一共多少通道，这里就是10了
    end
end