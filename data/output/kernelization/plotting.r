# Some variables that are used in various ways, to construct the plots
res_folder="./"
col_vec = c("darkorange")#,"red2","dodgerblue2","black", "purple")
pnt_vec = c(9)#,16,17,15,0)
columns  <- c('#sec','#it','#|V(G)|','#|E(G)|','#|V(Gk)|','#|E(Gk)|','#LOW_B(k)','#LOW_B(kk)','#MQLIB(G)','#MQLIB(Gk)','#locsearch(G)','#locsearch(Gk)','#locsearch_ext(Gk)','#locsearch_ext+MQLIB_OFF(Gk)', '#EE(G)', '#EE(Gk)', '#+kk', '#file')
 


#Need readjustment for each case:
x_start <- 10
#x_start_legend <- 9
case_type <- "ba_1024"
nam_vec = c("ba_1024")#, "original", "task a", "task b", "task c")
                                                            
# Read the results from the csv files
gnm_undirected  <- read.table(paste(res_folder, "out-avg"     , sep=""), comment.char = "#", col.names = columns)
#base_raw <- read.table(paste(res_folder, "hash_original.txt", sep=""), comment.char = "#", col.names = columns)
#a_raw    <- read.table(paste(res_folder, "hash_a.txt"       , sep=""), comment.char = "#", col.names = columns)
#b_raw    <- read.table(paste(res_folder, "hash_b.txt"       , sep=""), comment.char = "#", col.names = columns)
#c_raw    <- read.table(paste(res_folder, "hash_c.txt"       , sep=""), comment.char = "#", col.names = columns)

gnm_undirected$ratio_e = 1 - (gnm_undirected[,"X..E.Gk.."]/gnm_undirected[,"X..E.G.."])

#gnm_undirected$sum = rowSums(gnm_undirected[,c("X..R8.", "X..R9.", "X..R9x.","X..R10.")], na.rm=TRUE)
gnm_undirected[,"X..E.G.."] <- log(gnm_undirected[,"X..E.G.."] , 2)
#gnm_undirected <- transform(gnm_undirected, new = X..E.G.. / 2)

gnm_undirected <- gnm_undirected[with(gnm_undirected, order(X..E.G..)), ]
print(gnm_undirected)


# Aggregate the read data depending on the section number
#gnm_undirected  = aggregate(gnm_undirected , by=list(std_raw$sec) , FUN=mean)
#base = aggregate(base_raw, by=list(base_raw$sec), FUN=mean)
##a    = aggregate(a_raw   , by=list(a_raw$sec)   , FUN=mean)
#b    = aggregate(b_raw   , by=list(b_raw$sec)   , FUN=mean)
#c    = aggregate(c_raw   , by=list(c_raw$sec)   , FUN=mean)

# Open a PDF to store the plot into
pdf("plot.pdf", width=10, height=5)

print(gnm_undirected[,"X..V.G.."])

v_count <- gnm_undirected[,"X..V.G.."][1]
print(v_count)

for (entry in list(
    c("ratio_e", paste("Kernelization, ", case_type, "(|V|=2^", log(v_count, 2), ")", sep="")
    ))) {
    
    # Here we choose the two comlumns, that we use for the plot
    y=entry[1]
    x='X..E.G..'

    # Define some ranges for our plotting area
    xrange <- range(x_start,gnm_undirected[,x])
    yrange <- range(0, gnm_undirected[,y])
    yrange[2] <- yrange[2] * 1.1
    print(yrange)
    # Initialize the plotting area
    plot(xrange, yrange, yaxs='i', col="black", type="n", main="stuff", ann=FALSE)

    # Label titles for both axes
    title(xlab="number of edges, power of 2"     , line=2.3)
    title(ylab="kernelization efficiency =  1 - |E(G')| / |E(G)|", line=2.3)
    title(main=entry[2])

    # Draws the 4 lines of measurements
    lines(gnm_undirected[,x] , gnm_undirected[,y] , col=col_vec[[1]], type="o", pch=pnt_vec[[1]])

    # Shows the legend
    #legend(x_start_legend, yrange[2], nam_vec, lty=, col=col_vec, pch=pnt_vec)
}



# Closes the PDF
dev.off()
