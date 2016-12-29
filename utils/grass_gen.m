% Generate grass vertices in a plane
%% 7 vertex straight grass
FIG_SIZE = [250,150];

N = 7;
grass_1 = zeros(N,2);
width = 1;
height = 8;
x_hat = [1, 0];
y_hat = [0, 1];

interp_h = 0;

for index = 1:2:N-1
    % Magic numbers:
    interp_w = (width / 2 / index^0.25)^1.6;
    interp_h = (height - (height / index^0.2))^2;
    % x:
    grass_1(index, 1) = -interp_w;
    grass_1(index+1, 1) = interp_w;
    % y:
    grass_1(index, 2) = interp_h;
    grass_1(index+1, 2) = interp_h;
end

grass_1(7, :) = [0, height];
grass_1
fig1_name = 'Grass 1 LOD1: 7 vertices';
figure('Name', fig1_name, 'Position', [100, 100, FIG_SIZE]); 
plot(grass_1(2:2:end, 1), grass_1(2:2:end, 2), '-g')
title(fig1_name)
hold on;
plot(grass_1(1:2:end, 1), grass_1(1:2:end, 2), '-g')
plot(grass_1(2:2:end, 1), grass_1(2:2:end, 2), 'kx')
plot(grass_1(1:2:end, 1), grass_1(1:2:end, 2), 'kx')
hold off;
axis equal

disp('LOD1 - 7 vertices')
comma_sep = sprintf('%f, ' , grass_1(:,1)');
disp(['const float GRASS_1_X[7] = float[7](', comma_sep(1:end-2), ');'])
comma_sep = sprintf('%f, ' , grass_1(:,2)');
disp(['const float GRASS_1_Y[7] = float[7](', comma_sep(1:end-2), ');'])

disp('Grass 1 LOD2: 5 vertices')
grass_1(3:4, :) = [];
fig2_name = 'Grass 1 LOD2: 5 vertices';
figure('Name', fig2_name, 'Position', [100, 100 + 1.5 * FIG_SIZE(2), FIG_SIZE]); 
plot(grass_1(2:2:end, 1), grass_1(2:2:end, 2), '-g')
title(fig2_name)
hold on;
plot(grass_1(1:2:end, 1), grass_1(1:2:end, 2), '-g')
plot(grass_1(2:2:end, 1), grass_1(2:2:end, 2), 'kx')
plot(grass_1(1:2:end, 1), grass_1(1:2:end, 2), 'kx')
hold off;
axis equal

disp('LOD2 - 5 vertices')
comma_sep = sprintf('%f, ' , grass_1(:,1)');
disp(['const float GRASS_1_X[5] = float[5](', comma_sep(1:end-2), ');'])
comma_sep = sprintf('%f, ' , grass_1(:,2)');
disp(['const float GRASS_1_Y[5] = float[5](', comma_sep(1:end-2), ');'])

disp('Grass 1 LOD3: 3 vertices')
grass_1(3:4, :) = [];
fig3_name = 'Grass 1 LOD3: 3 vertices';
figure('Name', fig3_name, 'Position', [100, 100 + 1.5 * 2 * FIG_SIZE(2), FIG_SIZE]); 
plot(grass_1(2:2:end, 1), grass_1(2:2:end, 2), '-g')
title(fig3_name)
hold on;
plot(grass_1(1:2:end, 1), grass_1(1:2:end, 2), '-g')
plot(grass_1(2:2:end, 1), grass_1(2:2:end, 2), 'kx')
plot(grass_1(1:2:end, 1), grass_1(1:2:end, 2), 'kx')
hold off;
axis equal

disp('LOD3 - 3 vertices')
comma_sep = sprintf('%f, ' , grass_1(:,1)');
disp(['const float GRASS_1_X[5] = float[5](', comma_sep(1:end-2), ');'])
comma_sep = sprintf('%f, ' , grass_1(:,2)');
disp(['const float GRASS_1_Y[5] = float[5](', comma_sep(1:end-2), ');'])


%% 7 vertex bent grass
FIG_SIZE = [250,150];

N = 7;
grass_2 = zeros(N,2);
width = 1;
height = 8;
x_hat = [1, 0];
y_hat = [0, 1];

interp_h = 0;

for index = 1:2:6
    % Magic numbers:
    interp_w = (width / 2 / index^0.2)^1.2;
    interp_h = (height - (height / index^0.25))^2;
    bend_diff = 3 * (interp_h / height)^2.7;
    % x:
    grass_2(index, 1) = -interp_w + bend_diff;
    grass_2(index+1, 1) = interp_w + bend_diff;
    % y:
    grass_2(index, 2) = interp_h;
    grass_2(index+1, 2) = interp_h;
end

close all;
grass_2(N, :) = [width + 2, height];
grass_2
figure('Name', 'Bent grass 2: 7 vertices', 'Position', [200, 100, FIG_SIZE]);
plot(grass_2(2:2:end, 1), grass_2(2:2:end, 2), '-g')
for i= 1:7
    text(grass_2(i,1), grass_2(i,2), sprintf('%i',i))
end
hold on;
plot(grass_2(1:2:end, 1), grass_2(1:2:end, 2), '-g')
plot(grass_2(2:2:end, 1), grass_2(2:2:end, 2), 'kx')
plot(grass_2(1:2:end, 1), grass_2(1:2:end, 2), 'kx')
title('Bent grass: 7 vertices')
axis equal
hold off

% Using fill
figure('Name', 'FILL: Bent grass 2: 7 vertices', 'Position', [200 + FIG_SIZE(1), 100, FIG_SIZE]);
grass_matlab_style = [grass_2(1:2:end,:); grass_2(6:-2:2,:)];
fill(grass_matlab_style(:, 1), grass_matlab_style(:, 2), '-g*')
title('Bent grass: 7 vertices')
axis equal


disp('Bent grass 2, LOD1: 7 vertices')
comma_sep = sprintf('%f, ' , grass_2(:,1)');
disp(['const float GRASS_2_X[7] = float[7](', comma_sep(1:end-2), ');'])
comma_sep = sprintf('%f, ' , grass_2(:,2)');
disp(['const float GRASS_2_Y[7] = float[7](', comma_sep(1:end-2), ');'])

grass_2(N, :) = [width + 2, height];
grass_2(5:6,:) = (grass_2(3:4,:) + grass_2(5:6,:)) / 2;
grass_2(3:4,:) = [];
grass_2
figure('Name', 'Bent grass 2: 5 vertices', 'Position', [200, 100 + 1.5 * FIG_SIZE(2), FIG_SIZE])
plot(grass_2(2:2:end, 1), grass_2(2:2:end, 2), '-g')
hold on;
plot(grass_2(1:2:end, 1), grass_2(1:2:end, 2), '-g')
plot(grass_2(2:2:end, 1), grass_2(2:2:end, 2), 'kx')
plot(grass_2(1:2:end, 1), grass_2(1:2:end, 2), 'kx')
title('Bent grass: 5 vertices')
axis equal
hold off

% Using fill
figure('Name', 'FILL: Bent grass 2: 7 vertices', 'Position', [200 + FIG_SIZE(1), 100 + 1.5 * FIG_SIZE(2), FIG_SIZE]);
grass_matlab_style = [grass_2(1:2:end,:); grass_2(4:-2:2,:)];
fill(grass_matlab_style(:, 1), grass_matlab_style(:, 2), '-g*')
title('Bent grass: 5 vertices')
axis equal

disp('Bent grass 2, LOD2: 5 vertices')
comma_sep = sprintf('%f, ' , grass_2(:,1)');
disp(['const float GRASS_2_X[5] = float[5](', comma_sep(1:end-2), ');'])
comma_sep = sprintf('%f, ' , grass_2(:,2)');
disp(['const float GRASS_2_Y[5] = float[5](', comma_sep(1:end-2), ');'])

%% 7 vertex double grass
close all;
N = 7;
width = 2;
height = 4;
x_hat = [1, 0];
y_hat = [0, 1];

% Magic numbers:
x0 = -6 * width / 10;
x1 = -1 * width / 10;
x2 = -2 * width / 10;
x3 = 2 * width / 10;
x4 = 5 * width / 10;
x5 = 6.5 * width / 10;

y0 = 0;
y1 = 2.5 * height / 10;
y2 = 4.5 * height / 10;
y3 = height;

grass_3 = [x0, y2;
           x2, y1;
           x1, y0;
           x3, y0;
           x3, y2;
           x4, y2;
           x5, y3]

figure('Name', 'Chubby grass 3 LOD1: 7 vertices')
plot(grass_3(:, 1), grass_3(:, 2), '-gx')
for i= 1:7
    text(grass_3(i,1), grass_3(i,2), sprintf('%i',i))
end
axis equal

disp('Chubby grass 3, LOD1: 7 vertices')
comma_sep = sprintf('%f, ' , grass_3(:,1)');
disp(['const float GRASS_3_X[7] = float[7](', comma_sep(1:end-2), ');'])
comma_sep = sprintf('%f, ' , grass_3(:,2)');
disp(['const float GRASS_3_Y[7] = float[7](', comma_sep(1:end-2), ');'])

disp('Chubby grass 3, LOD2: 5 vertices')
grass_3(5:6,:) = [];

figure('Name', 'Chubby grass 3 LOD2: 5 vertices')
plot(grass_3(:, 1), grass_3(:, 2), '-gx')
for i= 1:5
    text(grass_3(i,1), grass_3(i,2), sprintf('%i',i))
end
axis equal

comma_sep = sprintf('%f, ' , grass_3(:,1)');
disp(['const float GRASS_3_X[5] = float[5](', comma_sep(1:end-2), ');'])
comma_sep = sprintf('%f, ' , grass_3(:,2)');
disp(['const float GRASS_3_Y[5] = float[5](', comma_sep(1:end-2), ');'])



