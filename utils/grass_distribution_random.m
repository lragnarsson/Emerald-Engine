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
 
 % Coordinates u,v along base vectors.
 % Conditions to be inside triangle:
 % 0 < u,v < 1
 % u + v < 1
 
 N = 10; 
 coords = zeros(N,2); 
 for i=1:N
    u = rand;
    v = rand;
    while u + v > 1
        u = rand;
        v = rand;
    end
    coords(i,:) = [u,v];
 end
 

 points = vertices(1,:) + coords * [base_vector_12; base_vector_13];
 
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