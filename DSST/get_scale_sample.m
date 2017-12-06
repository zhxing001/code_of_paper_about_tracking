function out = get_scale_sample(im, pos, base_target_sz, scaleFactors, scale_window, scale_model_sz)

% out = get_scale_sample(im, pos, base_target_sz, scaleFactors, scale_window, scale_model_sz)
% 
% Extracts a sample for the scale filter at the current
% location and scale.

nScales = length(scaleFactors);

for s = 1:nScales
    patch_sz = floor(base_target_sz * scaleFactors(s));    
    %scaleFactors里面存的是不同的放大比例，33维，比如从1.5--0.5之间的，但不是线性的
    %
    xs = floor(pos(2)) + (1:patch_sz(2)) - floor(patch_sz(2)/2);
    ys = floor(pos(1)) + (1:patch_sz(1)) - floor(patch_sz(1)/2);
    %这里根据pos和patch得到xs和ys的索引范围。
    
    % check for out-of-bounds coordinates, and set them to the values at
    % the borders
    %处理边界
    xs(xs < 1) = 1;
    ys(ys < 1) = 1;
    xs(xs > size(im,2)) = size(im,2);
    ys(ys > size(im,1)) = size(im,1);
    
    % extract image   得到当前尺寸下的图像
    im_patch = im(ys, xs, :);
    
    % resize image to model size
    % 重构统一的大小，这里的scale_model_sz是根据一开始的目标大小定的，见dsst函数，这里是19*26
    im_patch_resized = mexResize(im_patch, scale_model_sz, 'auto');
    
    % extract scale features
    % 得到31维的hog特征，19/4*26/4*31=4*6*31=744维的特征，这里的fhog是串联起来了，并没有并联
    temp_hog = fhog(single(im_patch_resized), 4);  %cell是4*4的
    temp = temp_hog(:,:,1:31);      %只取了前31维的hog，最后一维是灰度，并没有要    
    
    if s == 1                 %如果是第一个尺度，创建一个out来存这个特征
        out = zeros(numel(temp), nScales, 'single');
    end
    
    % window
    out(:,s) = temp(:) * scale_window(s);
end