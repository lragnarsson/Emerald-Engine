% Grass distribution on a polygon

% View space coordinates of triangle

vertices = ...
    [10, 12, 14; ...
     15, 14, 13; ...
     12, 10, 10];
 vertices(4,:) = vertices(1,:);
 
 % Don't normalize the base vectors 
 base_vector_12 = (vertices(2,:) - vertices(1,:));
 base_vector_13 = (vertices(3,:) - vertices(1,:));
 
 % Barycentric coordinates u,v along base vectors.
 % Conditions to be inside triangle:
 % 0 < u,v < 1
 % u + v < 1
 
 rows = 3;
 N = (rows + 1) / 2 * rows;
 coords = zeros(N,2); 
 i = 1;
 step = 1 / (4 * (rows + 1));
 for r=rows:-1:1
    for j=1:rows + 1 - r
        coords(i,:) = [step * (5*j - 3), step * (5*r - 3)];
        i = i+1;
    end
 end
 

 points = repmat(vertices(1,:), N, 1) + coords * [base_vector_12; base_vector_13];
 
 fig = figure(1);
 fig.Name = 'Grass distribution on triangle';
 hold off
 plot3(vertices(:,1), vertices(:,2), vertices(:,3))
 hold on
 grid on
 text(vertices(1,1), vertices(1,2), vertices(1,3), 'p1')
 text(vertices(2,1), vertices(2,2), vertices(2,3), 'p2')
 text(vertices(3,1), vertices(3,2), vertices(3,3), 'p3')
  
  for i=1:N
      scatter3(points(i,1), points(i,2), points(i,3), 'r', 'filled');
  end
  
  disp('Coordinates u,v along base vectors. Ready to copy paste into shader.');
  comma_sep = sprintf('%f, ' , coords(:,1)');
  name_u = sprintf('const float COORDS_U[%i] = float[%i](', rows*2, rows*2);
  name_v = sprintf('const float COORDS_V[%i] = float[%i](', rows*2, rows*2);
  disp([name_u, comma_sep(1:end-2), ');'])
  comma_sep = sprintf('%f, ' , coords(:,2)');
  disp([name_v, comma_sep(1:end-2), ');'])
