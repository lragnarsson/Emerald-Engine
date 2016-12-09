% Generate grass vertices in a plane
%% 9 vertex straight grass
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
plot(grass_1(2:2:end, 1), grass_1(2:2:end, 2), '-gx')
hold on;
plot(grass_1(1:2:end, 1), grass_1(1:2:end, 2), '-gx')
hold off;
axis equal

comma_sep = sprintf('%f, ' , grass_1(:,1)');
disp(['const float GRASS_1_X[7] = float[7](', comma_sep(1:end-2), ');'])
comma_sep = sprintf('%f, ' , grass_1(:,2)');
disp(['const float GRASS_1_Y[7] = float[7](', comma_sep(1:end-2), ');'])


%% 7 vertex bent grass
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

grass_2(N, :) = [width + 2, height];
grass_2
plot(grass_2(2:2:end, 1), grass_2(2:2:end, 2), '-gx')
hold on;
plot(grass_2(1:2:end, 1), grass_2(1:2:end, 2), '-gx')
axis equal
hold off

comma_sep = sprintf('%f, ' , grass_2(:,1)');
disp(['const float GRASS_2_X[7] = float[7](', comma_sep(1:end-2), ');'])
comma_sep = sprintf('%f, ' , grass_2(:,2)');
disp(['const float GRASS_2_Y[7] = float[7](', comma_sep(1:end-2), ');'])


%% 7 vertex double grass
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

plot(grass_3(:, 1), grass_3(:, 2), '-gx')
axis equal

comma_sep = sprintf('%f, ' , grass_3(:,1)');
disp(['const float GRASS_3_X[7] = float[7](', comma_sep(1:end-2), ');'])
comma_sep = sprintf('%f, ' , grass_3(:,2)');
disp(['const float GRASS_3_Y[7] = float[7](', comma_sep(1:end-2), ');'])






