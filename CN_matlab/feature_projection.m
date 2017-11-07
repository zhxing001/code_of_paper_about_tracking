function z = feature_projection(x_npca, x_pca, projection_matrix, cos_window)
%特征投影
% z = feature_projection(x_npca, x_pca, projection_matrix, cos_window)
%
% Calculates the compressed feature map by mapping the PCA features with
% the projection matrix and concatinates this with the non-PCA features.
% The feature map is then windowed.

if isempty(x_pca)
    % if no PCA-features exist, only use non-PCA
    z = x_npca;      %如果是灰度图的话就不用处理了，直接赋值就可以
else
    % get dimensions
    [height, width] = size(cos_window);    %余弦窗大小，这个和padding之后的sz一样大
    [num_pca_in, num_pca_out] = size(projection_matrix);   %投影矩阵是10*2的，可以分别作为pca的输入和输出维数
    
    % project the PCA-features using the projection matrix and reshape
    % to a window
    x_proj_pca = reshape(x_pca * projection_matrix, [height, width, num_pca_out]);    %投影矩阵乘上去，这样就是二维的特征
    
    % concatinate the feature windows
    if isempty(x_npca)
        z = x_proj_pca;
    else
        z = cat(3, x_npca, x_proj_pca);   %链接到一起，最终这个是1一个灰度，二个颜色
    end
end

% do the windowing of the output
z = bsxfun(@times, cos_window, z);
end