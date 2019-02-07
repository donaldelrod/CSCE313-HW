stats = load('client_stats.dat');

buffersize = sortrows(stats, [2 4]);
workers = sortrows(stats, [3 4]);
time = sortrows(stats, 4);

figure(1)
scatter3(workers(:,3), workers(:,4), workers(:,2), 10, workers(:,2), 'filled');
xlabel('Time (s)');
ylabel('Number of workers');
zlabel('Size of Buffer');

saveas( gcf(), 'workers_vs_time_vs_bufsize.png', 'png');

figure(2)
scatter3(buffersize(:,4), buffersize(:,2), buffersize(:,3), 10, buffersize(:,3), 'filled');
xlabel('Time (s)');
ylabel('Buffer size');
zlabel('Number of workers');

saveas( gcf(), 'bufsize_vs_time_vs_workers.png', 'png');

figure(3)
scatter(time(:,4), time(:,2));
xlabel('Time (s)')
ylabel('Buffer size')

saveas( gcf(), 'bufsize_vs_time.png', 'png');

for i = 1:40
    temp = workers(:,3) == i;
    temp = workers(temp, :);
    av = mean(temp(:, 4));
    fprintf('%i workers averages %.5f seconds\n', i, av);
end

exit;