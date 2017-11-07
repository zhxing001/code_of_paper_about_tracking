function [positions, fps] = color_tracker(params)

% [positions, fps] = color_tracker(params)

% parameters
padding = params.padding;
output_sigma_factor = params.output_sigma_factor;
sigma = params.sigma;
lambda = params.lambda;
learning_rate = params.learning_rate;
compression_learning_rate = params.compression_learning_rate;
non_compressed_features = params.non_compressed_features;
compressed_features = params.compressed_features;
num_compressed_dim = params.num_compressed_dim;     %这里要降到2维

video_path = params.video_path;
img_files = params.img_files;
pos = floor(params.init_pos);
target_sz = floor(params.wsize);

visualization = params.visualization;

num_frames = numel(img_files);

% load the normalized Color Name matrix
temp = load('w2crs');
w2c = temp.w2crs;

use_dimensionality_reduction = ~isempty(compressed_features);

% window size, taking padding into account
sz = floor(target_sz * (1 + padding));

% desired output (gaussian shaped), bandwidth proportional to target size
output_sigma = sqrt(prod(target_sz)) * output_sigma_factor;
[rs, cs] = ndgrid((1:sz(1)) - floor(sz(1)/2), (1:sz(2)) - floor(sz(2)/2));
y = exp(-0.5 / output_sigma^2 * (rs.^2 + cs.^2));
yf = single(fft2(y));

% store pre-computed cosine window
cos_window = single(hann(sz(1)) * hann(sz(2))');        %余弦窗

% to calculate precision
positions = zeros(numel(img_files), 4);         %储存位置计算准确率

% initialize the projection matrix
projection_matrix = [];          %初始化投影矩阵？

% to calculate fps
time = 0;       %计时

for frame = 1:num_frames,
    % load image
    im = imread([video_path img_files{frame}]);

    tic;
    
    if frame > 1
        % compute the compressed learnt appearance
        zp = feature_projection(z_npca, z_pca, projection_matrix, cos_window);
        
        % extract the feature map of the local image patch
        [xo_npca, xo_pca] = get_subwindow(im, pos, sz, non_compressed_features, compressed_features, w2c);
        
        % do the dimensionality reduction and windowing
        x = feature_projection(xo_npca, xo_pca, projection_matrix, cos_window);
        
        % calculate the response of the classifier
        kf = fft2(dense_gauss_kernel(sigma, x, zp));
        response = real(ifft2(alphaf_num .* kf ./ alphaf_den));    %这个也和CSK稍有不同，主要是alpha由两部分组成了
        
        % target location is at the maximum response
        [row, col] = find(response == max(response(:)), 1);
        pos = pos - floor(sz/2) + [row, col];
    end
    
    % extract the feature map of the local image patch to train the classifer
    %这里得到的是两个特征，一组是nopca是一个灰度，另一组是xo-pca是一个十维的
    %xo_npca是sz大小的灰度，xo_pca是一个[sz(1)*sz(2),10]大小的，是一个十维的，而且是重构的，不是原图像的大小，长和宽乘起来，相当于拉长了
    [xo_npca, xo_pca] = get_subwindow(im, pos, sz, non_compressed_features, compressed_features, w2c);
    
    if frame == 1
        % initialize the appearance
        z_npca = xo_npca;
        z_pca = xo_pca;
        
        % set number of compressed dimensions to maximum if too many
        num_compressed_dim = min(num_compressed_dim, size(xo_pca, 2));
    else
        % update the appearance
        z_npca = (1 - learning_rate) * z_npca + learning_rate * xo_npca;
        z_pca = (1 - learning_rate) * z_pca + learning_rate * xo_pca;
    end
    
    % if dimensionality reduction is used: update the projection matrix
    if use_dimensionality_reduction     %这里是搞pca的
        % compute the mean appearance   %均值
        data_mean = mean(z_pca, 1);
        
        % substract the mean from the appearance to get the data matrix
        data_matrix = bsxfun(@minus, z_pca, data_mean);       %做差，即中心化
        
        % calculate the covariance matrix
        cov_matrix = 1/(prod(sz) - 1) * (data_matrix' * data_matrix);   %协方差矩阵
        
        % calculate the principal components (pca_basis) and corresponding variances
        if frame == 1
            [pca_basis, pca_variances, ~] = svd(cov_matrix);        %SVD分解
        else
            [pca_basis, pca_variances, ~] = svd((1 - compression_learning_rate) * old_cov_matrix + compression_learning_rate * cov_matrix);  %插值之后做SVD分解
        end
        
        % calculate the projection matrix as the first principal
        % components and extract their corresponding variances
        projection_matrix = pca_basis(:, 1:num_compressed_dim);    %取前两维作为投影矩阵
        %下面这个是一点更新策略
        projection_variances = pca_variances(1:num_compressed_dim, 1:num_compressed_dim);
        
        if frame == 1
            % initialize the old covariance matrix using the computed
            % projection matrix and variances
            old_cov_matrix = projection_matrix * projection_variances * projection_matrix';
        else
            % update the old covariance matrix using the computed
            % projection matrix and variances
            old_cov_matrix = (1 - compression_learning_rate) * old_cov_matrix + compression_learning_rate * (projection_matrix * projection_variances * projection_matrix');
        end
    end
    
    % project the features of the new appearance example using the new
    % projection matrix
    x = feature_projection(xo_npca, xo_pca, projection_matrix, cos_window);
    
    % calculate the new classifier coefficients  算新分类器的系数
    kf = fft2(dense_gauss_kernel(sigma, x));
    new_alphaf_num = yf .* kf;
    new_alphaf_den = kf .* (kf + lambda);    %训练，这里分别是分子和分母，下面还有更新，见论文(5)
    
    if frame == 1
        % first frame, train with a single image
        alphaf_num = new_alphaf_num;
        alphaf_den = new_alphaf_den;
    else
        % subsequent frames, update the model
        alphaf_num = (1 - learning_rate) * alphaf_num + learning_rate * new_alphaf_num;     %这是一个分子
        alphaf_den = (1 - learning_rate) * alphaf_den + learning_rate * new_alphaf_den;     %这是分母    同时更新分子和分母，见论文的（5）式
    end
    
    %save position
    positions(frame,:) = [pos target_sz];
    
    time = time + toc;
    
    %visualization
    if visualization == 1
        rect_position = [pos([2,1]) - target_sz([2,1])/2, target_sz([2,1])];
        if frame == 1,  %first frame, create GUI
            figure('Name',['Tracker - ' video_path]);
            im_handle = imshow(uint8(im), 'Border','tight', 'InitialMag', 100 + 100 * (length(im) < 500));
            rect_handle = rectangle('Position',rect_position, 'EdgeColor','g');
            text_handle = text(10, 10, int2str(frame));
            set(text_handle, 'color', [0 1 1]);
        else
            try  %subsequent frames, update GUI
                set(im_handle, 'CData', im)
                set(rect_handle, 'Position', rect_position)
                set(text_handle, 'string', int2str(frame));
            catch
                return
            end
        end
        
        drawnow
%         pause
    end
end

fps = num_frames/time;
