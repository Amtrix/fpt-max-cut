# Some variables that are used in various ways, to construct the plots
res_folder="./"
col_vec = c("darkorange", "red2","dodgerblue2","black", "purple", "blue", "yellow")
pnt_vec = c(9,16,17,15,0, 5, 2)
columns  <- c('V','CEXT','coverage')
 


#Need readjustment for each case:
x_start <- 0
x_start_legend <- 0.9
nam_vec = c("|V| = 2", "|V| = 3", "|V| = 4", "|V| = 5", "|V| = 6", "|V| = 7")
                                                            
# Read the results from the csv files
data_table  <- read.table(paste(res_folder, "aggregate"     , sep=""), comment.char = "#", col.names = columns)



data_table$ratio = (data_table[,"CEXT"]/data_table[,"V"])
print(subset(data_table, V==2))

#data_table$sum = rowSums(data_table[,c("X..R8.", "X..R9.", "X..R9x.","X..R10.")], na.rm=TRUE)
#data_table[,"X..E.G.."] <- log(data_table[,"X..E.G.."] , 2)
#data_table <- transform(data_table, new = X..E.G.. / 2)

#data_table <- data_table[with(data_table, order(X..E.G..)), ]
#print(data_table)


# Aggregate the read data depending on the section number
#data_table  = aggregate(data_table , by=list(std_raw$sec) , FUN=mean)
#base = aggregate(base_raw, by=list(base_raw$sec), FUN=mean)
##a    = aggregate(a_raw   , by=list(a_raw$sec)   , FUN=mean)
#b    = aggregate(b_raw   , by=list(b_raw$sec)   , FUN=mean)
#c    = aggregate(c_raw   , by=list(c_raw$sec)   , FUN=mean)

# Open a PDF to store the plot into
pdf("plot.pdf", width=10, height=5)


for (entry in list(
    c("ratio_e", paste("Kernelization coverage", "", sep=""), 2)
    
    
    )) {
    
    # Here we choose the two comlumns, that we use for the plot
    y='coverage'
    x='ratio'

    # Define some ranges for our plotting area
    xrange <- range(x_start,data_table[,x])
    yrange <- range(0, data_table[,y])
    yrange[2] <- yrange[2] * 1.1
    print(yrange)
    # Initialize the plotting area
    plot(xrange, yrange, yaxs='i', col="black", type="n", main="stuff", ann=FALSE)

    # Label titles for both axes
    title(xlab="|Cext|/|V|"     , line=2.3)
    title(ylab="coverage", line=2.3)
    title(main=entry[2])

    # Draws the 4 lines of measurements
    for (dx in 2:7) {
        subtable <- subset(data_table, V==dx)
        look_dx <- dx - 1
        lines(subtable[,x] , subtable[,y] , col=col_vec[[look_dx]], type="o", pch=pnt_vec[[look_dx]])
    }

    # Shows the legend
    legend(x_start_legend, 0.6, nam_vec, lty=, col=col_vec, pch=pnt_vec)
}



# Closes the PDF
dev.off()
