% Tested in octave. It should be running in matlab too.
clear all
close all

files = dir('../out/build/x64-Release/bench/*.csv')
cs=[];
ins=[];
prc=[];
for j = 1 : length( files )
    filename = [files(j).folder '/' files(j).name]
    
    
    container_size = dlmread( filename, ',',[0,1,0,1]);
    a=dlmread( filename, ',', 2, 1);
    
    insertions = a(1:7,1)';
    processing = a(8:end,1)';
    
    cs=[cs; container_size];
    ins=[ins;insertions];
    prc=[prc;processing];
end

[cs, ord] = sort(cs);
ins = ins(ord,:);
prc= prc(ord,:);

figure(1)
hb = bar(ins);
num_x_vals = numel(hb);
dx = get(hb(1),'BarWidth')/num_x_vals;
leftEdge = get(hb(1),'XData') - get( hb(1),'BarWidth')/2;
leftEdge = leftEdge + dx/2;

set(gca,'XTickLabel', num2str(cs))
xlabel('Array size');
ylabel('Time (ns)');

ymax=550;
ylim([0,ymax])
for k = 1:length(hb)
    y= get(hb(k),'YData')

    x = leftEdge + (k-1)*dx;
    
    for i = 1 : length(y)
        if ( y(i) >= 9.5 )
            label = num2str( round(y(i)), '%3.0f');
        else % We can afford small numbers on top of the bars
            label = num2str( round(y(i)), '%2.1f');
        end
        
        text(x(i),y(i)+5,label,...
            'HorizontalAlignment','center',...
            'VerticalAlignment','bottom');
    end
end

legend( 'std::vector of pointers', ...
    'std::variant vector',...
    'boost::poly\_collection (restitution)', ...
    'boost::any\_collection (restitution)', ...
    'eumorphic::ordered\_collection (heap)', ...
    'eumorphic::collection (heap)', ...
    'eumorphic::collection (stack)' )
set(gcf, 'Position', [50, 50, 1270, 547 ])

title ( [ 'Per element insertion time']);
text( 0.1, ymax*0.98, 'Lower is better','VerticalAlignment','Top' );

set(gcf,'PaperPosition', [0.25, 2.5, 18, 6] )

drawnow

print( [ 'hetero_insertion_benchmark.png' ],'-r300','-dpng')
print( [ 'hetero_insertion_benchmark.pdf' ],'-dpdf')



figure(2)
hb = bar(prc);
num_x_vals = numel(hb);
dx = get(hb(1),'BarWidth')/num_x_vals;
leftEdge = get(hb(1),'XData') - get( hb(1),'BarWidth')/2;
leftEdge = leftEdge + dx/2;

set(gca,'XTickLabel', num2str(cs))
xlabel('Array size');
ylabel('Time (ns)');

ymax = 120;
ylim([0,ymax])

for k = 1:length(hb)
    y= get(hb(k),'YData')

    x = leftEdge + (k-1)*dx;
    
    for i = 1 : length(y)
        if ( y(i) >= 9.5 )
            label = num2str( round(y(i)), '%3.0f');
        else % We can afford small numbers on top of the bars
            label = num2str( round(y(i)), '%2.1f');
        end
        
        text(x(i),y(i)+2,label,...
            'HorizontalAlignment','center',...
            'VerticalAlignment','bottom');
    end
end

legend( 'std::vector of pointers', ...
    'std::variant vector',...
    'boost::poly\_collection (restitution)', ...
    'boost::any\_collection (restitution)', ...
    'eumorphic::ordered\_collection (heap)', ...
    'eumorphic::collection (heap)', ...
    'eumorphic::collection (stack)' )
set(gcf, 'Position', [50, 50, 1270, 547 ])

title ( [ 'Per element processing time']);
text( 0.1, ymax*0.98, 'Lower is better','VerticalAlignment','Top' );


set(gcf,'PaperPosition', [0.25, 2.5, 18, 6] )

print( [ 'hetero_access_benchmark.png' ],'-r300','-dpng')
print( [ 'hetero_access_benchmark.pdf' ],'-dpdf')
