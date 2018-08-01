# Some variables that are used in various ways, to construct the plots
res_folder="./"
col_vec = c("darkorange")#,"red2","dodgerblue2","black", "purple")
pnt_vec = c(9)#,16,17,15,0)
nam_vec = c("gnm_undirected")#, "original", "task a", "task b", "task c")
columns  <- c('#|V|','#|E|','#|R8|','#|R9|','#|R9x|','#|R10|','#file')
                                                            
# Read the results from the csv files
gnm_undirected  <- read.table(paste(res_folder, "gnm_undirected"     , sep=""), comment.char = "#", col.names = columns)
#base_raw <- read.table(paste(res_folder, "hash_original.txt", sep=""), comment.char = "#", col.names = columns)
#a_raw    <- read.table(paste(res_folder, "hash_a.txt"       , sep=""), comment.char = "#", col.names = columns)
#b_raw    <- read.table(paste(res_folder, "hash_b.txt"       , sep=""), comment.char = "#", col.names = columns)
#c_raw    <- read.table(paste(res_folder, "hash_c.txt"       , sep=""), comment.char = "#", col.names = columns)

print(gnm_undirected)

gnm_undirected$sum = rowSums(gnm_undirected[,c("X..R8.", "X..R9.", "X..R9x.","X..R10.")], na.rm=TRUE)
gnm_undirected[,"X..E."] <- log(gnm_undirected[,"X..E."] , 2)

print(gnm_undirected)

# Aggregate the read data depending on the section number
#gnm_undirected  = aggregate(gnm_undirected , by=list(std_raw$sec) , FUN=mean)
#base = aggregate(base_raw, by=list(base_raw$sec), FUN=mean)
##a    = aggregate(a_raw   , by=list(a_raw$sec)   , FUN=mean)
#b    = aggregate(b_raw   , by=list(b_raw$sec)   , FUN=mean)
#c    = aggregate(c_raw   , by=list(c_raw$sec)   , FUN=mean)

# Open a PDF to store the plot into
pdf("plot.pdf", width=10, height=5)



for (entry in list(
    c("sum","Initial kernelization applicability count, gnm_undirected:")
    )) {
    
    # Here we choose the two comlumns, that we use for the plot
    y=entry[1]
    x='X..E.'

    # Define some ranges for our plotting area
    xrange <- range(0,9,gnm_undirected[,x])
    yrange <- range(0, gnm_undirected[,y])
    yrange[2] <- yrange[2] * 1.1
    print(yrange)
    # Initialize the plotting area
    plot(xrange, yrange, yaxs='i', col="black", type="n", main="stuff", ann=FALSE)

    # Label titles for both axes
    title(xlab="edge count, power of 2"     , line=2.3)
    title(ylab="# of applicable rules", line=2.3)
    title(main=entry[2])

    # Draws the 4 lines of measurements
    lines(gnm_undirected[,x] , gnm_undirected[,y] , col=col_vec[[1]], type="o", pch=pnt_vec[[1]])

    # Shows the legend
    legend(0, yrange[2], nam_vec, lty=1, col=col_vec, pch=pnt_vec)
}



# Closes the PDF
dev.off()
