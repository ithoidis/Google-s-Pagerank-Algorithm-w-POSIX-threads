% A simple demo of google's pagerank algorithm.
% It uses the pagerank implementation provided 
% by Cleve Moler and MathWorks
%
% author: Nikos Sismanis
% date: Jan 2014

% create a small data set of web pages
[U, G] = surfer('http://www.google.com/', 100);


% Eliminate any self-referential links
G = G - diag(diag(G));


[n,n] = size(G);
for j = 1:n
   L{j} = find(G(:,j));
   c(j) = length(L{j});
end

% write array to file
    dlmwrite('myFile.txt',100)
    dlmwrite('myFile.txt',c(1)','-append', 'delimiter','\t')
    dlmwrite('myFile.txt',L{1}','-append', 'delimiter','\t')
for j = 2:n
    dlmwrite('myFile.txt',c(j),'-append', 'delimiter','\t')
    dlmwrite('myFile.txt',L{j}','-append', 'delimiter','\t')
end

system('make');
system('./a.out 4');
% Use the power method to compute the eigenvector that correspond to the
% largest eigenvalue (Ranks)
x = pagerankpow(G);

% plot the ranks
figure
bar(x)
title('Page Ranks')


