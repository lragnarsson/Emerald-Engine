% Generate grass vertices in a plane
%% 9 vertex straight grass
N = 9;
grass_1 = zeros(N,2);
width = 1;
height = 20;
x_hat = [1, 0];
y_hat = [0, 1];

interp_h = 0;

for index = 1:2:8
    % Magic numbers:
    interp_w = (width / 2 / index^0.25)^1.6;
    interp_h = (height - (height / index^0.2))^1.5;
    % x:
    grass_1(index, 1) = -interp_w;
    grass_1(index+1, 1) = interp_w;
    % y:
    grass_1(index, 2) = interp_h;
    grass_1(index+1, 2) = interp_h;
end

grass_1(9, :) = [0, height];
grass_1
plot(grass_1(2:2:end, 1), grass_1(2:2:end, 2), '-gx')
hold on;
plot(grass_1(1:2:end, 1), grass_1(1:2:end, 2), '-gx')
hold off;
xlim([-height/2, height/2])
ylim([0, height])

comma_sep = sprintf('%f, ' , grass_1(:,1)');
disp(['const float grass_1_x[9] = float[9](', comma_sep(1:end-2), ');'])
comma_sep = sprintf('%f, ' , grass_1(:,2)');
disp(['const float grass_1_y[9] = float[9](', comma_sep(1:end-2), ');'])

%%




