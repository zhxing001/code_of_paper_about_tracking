function video_path = choose_video(base_path)
%CHOOSE_VIDEO
%   Allows the user to choose a video (sub-folder in the given path).
%   Returns the full path to the selected sub-folder.
   %返回目标视频的完整路径，中间处理还是略麻烦，不过为了程序更友好就只能这样写了。
   %170905  加入详细中文注释。
%
%   Jo鉶 F. Henriques, 2012
%   http://www.isr.uc.pt/~henriques/

	%process path to make sure it's uniform
	if ispc(), base_path = strrep(base_path, '\', '/'); end
    %把base_path里的  \  全部换成  /；这是matlab可识别的路径
	if base_path(end) ~= '/', base_path(end+1) = '/'; end
	%如果路径最后一个不是 /  ，就在后面加上  /
    
	%list all sub-folders
	contents = dir(base_path);   
    %获得所有的子文件，返回一个结构体
	names = {};
	for k = 1:numel(contents), %文件的个数
		name = contents(k).name;      %文件名存入name细胞组
		if isdir([base_path name]) && ~strcmp(name, '.') && ~strcmp(name, '..'),
            
            %isdir:判断路径里是否是文件夹，如果是，返回true；
            %strcmp:判断两个字符串是否相同,如果相同,返回TRUE;
            %如果data不在根文件夹下，总会有两个特殊的文件，分别时  .  he  .. ，这句的意思就是
            %要排除吊这两个，并把文件名存入names细胞组里。
			names{end+1} = name;  %#ok  扩充names并存入name
		end
	end
	
	%no sub-folders found
	if  isempty(names),
        video_path = []; 
        return;
    end
    
	
	%choice GUI  %选择图形界面
	choice = listdlg('ListString',names, 'Name','Choose video', 'SelectionMode','single');
	%创建矩形框，让用户选择，单选，选择names下的，这个去看函数帮助，讲的很清楚
	if isempty(choice),  %user cancelled   
		video_path = [];
        %如果没有选择，video_path就为空
	else
		video_path = [base_path names{choice} '/'];
        %如果选择了，就把那个文件夹传入并在其后加 /
	end
	
end


