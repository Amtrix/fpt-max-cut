#!/usr/bin/env Rscript

options("width"=230)
library("optparse")
library(dplyr)

option_list = list(
    make_option(c("-f", "--file"), type="character", default=NULL, 
              help="dataset file name", metavar="character"),

	make_option(c("-o", "--out"), type="character", default=NULL, 
              help="output file name [default= %default]", metavar="character"),
    
    make_option(c("-m", "--mode"), type="character", default=NULL, 
              help="mode: iso-rem or iso-kept", metavar="character")
); 

opt_parser = OptionParser(option_list=option_list);
opt = parse_args(opt_parser);

if (is.null(opt$file) || is.null(opt$out) || is.null(opt$mode)){
    print_help(opt_parser)
    stop("At least one argument must be supplied (input file).n", call.=FALSE)
}



# Some variables that are used in various ways, to construct the plots
res_folder="./"
col_vec = c("darkorange", "red2","dodgerblue2","black", "purple", "blue", "yellow")
pnt_vec = c(9,18,17,15,0, 5, 2)
columns  <- c('V','CEXT','numgraphs','numclasses','coverage')
 


#Need readjustment for each case:
x_start <- 0
x_start_legend <- 0.9
nam_vec = c("|V| = 2", "|V| = 3", "|V| = 4", "|V| = 5", "|V| = 6", "|V| = 7")
if (opt$mode == "iso-kept")
    nam_vec = c("|V| = 2", "|V| = 3", "|V| = 4", "|V| = 5", "|V| = 6")
                                                            
# Read the results from the csv files
data_table  <- read.table(paste(res_folder, opt$file     , sep=""), comment.char = "#", col.names = columns)



data_table$ratio = (data_table[,"CEXT"])#/data_table[,"V"])
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
pdf(opt$out, width=10, height=5)

caption <- "Kernelization coverage - isomorphisms removed"
if (opt$mode == "iso-kept")
    caption <- "Kernelization coverage - isomorphisms kept"


for (entry in list(
    c("ratio_e", paste(caption, "", sep=""), 2)
    
    
    )) {
    
    # Here we choose the two comlumns, that we use for the plot
    y='coverage'
    x='ratio'

    # Define some ranges for our plotting area
    xrange <- range(x_start,data_table[,x])
    yrange <- range(0, data_table[,y])
    yrange[2] <- 1
    yrange[1] <- 0
    xrange[0] = 0
    print(yrange)
    # Initialize the plotting area
    #plot(xrange, yrange, yaxs='i', col="black", type="n", main="stuff", ann=FALSE)
    plot(xrange, yrange, yaxt='n', xaxs='i', yaxs='i', col="black", type="n", main="stuff", ann=FALSE)
    axis(2, at=pretty(yrange), lab=paste0(pretty(yrange) * 100, '%'), las=TRUE)

    # Label titles for both axes
    title(xlab="Number of external vertices in subgraph"     , line=2.3)
    title(ylab="coverage", line=3.3)
    title(main=entry[2])

    # Draws the 4 lines of measurements
    for (dx in 2:(length(nam_vec)+1)) {
        subtable <- subset(data_table, V==dx)
        look_dx <- dx - 1
        lines(subtable[,x] , subtable[,y] , col=col_vec[[look_dx]], type="o", pch=pnt_vec[[look_dx]])
    }

    # Shows the legend
    legend("bottomright", 0.6, nam_vec, lty=, col=col_vec, pch=pnt_vec)
}



# Closes the PDF
dev.off()
