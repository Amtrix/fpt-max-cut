#!/usr/bin/env Rscript

options("width"=230)
options(scipen = 999)
library("optparse")
suppressWarnings(suppressMessages(library("dplyr")))

option_list = list(
    make_option(c("-f", "--file"), type="character", default=NULL, 
              help="dataset file name", metavar="character"),


	make_option(c("-o", "--out"), type="character", default=NULL, 
              help="output file name [default= %default]", metavar="character"),

    make_option(c("--loess"), type="double", default=NULL, 
              help="loess factor, turned off if not provided", metavar="double"),

    make_option(c("--nopoints"), action="store_true", type="logical", default=FALSE, 
              help="If points should be omitted", metavar="logical")
); 

opt_parser = OptionParser(option_list=option_list);
opt = parse_args(opt_parser);

if (is.null(opt$file) || is.null(opt$out)){
    print_help(opt_parser)
    stop("At least one argument must be supplied (input file).n", call.=FALSE)
}

printloess = TRUE
loessv = 0.01;
if (is.null(opt$loess)) {
    printloess = FALSE
} else {
    loessv = 0.01;
}

loessv <- 0.01
eliminate_in_kernelized_too <- TRUE


res_folder=""
col_vec = c("darkorange","red2","dodgerblue2","black",
            "red","purple", "red","darkgreen","mediumorchid1", "aquamarine2")
pnt_vec  = c(18,4,18,15,0,18,4,18,15,0)
pnt_vec1 = c(pnt_vec[[1]], pnt_vec[[1]], pnt_vec[[1]], pnt_vec[[1]], pnt_vec[[1]], pnt_vec[[1]], pnt_vec[[1]], pnt_vec[[1]], pnt_vec[[1]], pnt_vec[[1]], pnt_vec[[1]], pnt_vec[[1]], pnt_vec[[1]], pnt_vec[[1]], pnt_vec[[1]])
columns <- c("sec", "numnodes","numedges","timex","maxcutsz","file")

data_table <- read.table(paste(res_folder, opt$file, sep=""), comment.char = "#", col.names = columns)
data_table_k <- read.table(paste(res_folder, opt$file, "-kernelized", sep=""), comment.char = "#", col.names = columns)



datasets <- unique(data_table[,"file"])

GetDatasetId <- function(name) {
    dx <- 0
    for (i in datasets) {
        if (i == name) {
            return(dx)
        }
        dx <- dx + 1
    }
    return(-2)
}

#data_table$sec <- GetDatasetId(data_table[,"file"])
data_table$xsec <- lapply(data_table[,"file"], function(x) GetDatasetId(x))
data_table_k$xsec <- lapply(data_table_k[,"file"], function(x) GetDatasetId(x))
#data_table$timex <- data_table$timex * 1.01

#data_table <- dplyr::filter(data_table, timex < 600)
#data_table_k <- dplyr::filter(data_table_k, timex < 600)


for (i in 0:(length(datasets)-1)) {
    sub <- dplyr::filter(data_table, xsec == i)
    subk <- dplyr::filter(data_table_k, xsec == i)
    #subnonzero <- dplyr::filter(sub, maxcutsz != 0)
    #hasfailed <- length(subnonzero$maxcutsz) == 0
    mx <- max(sub$maxcutsz)
    mxk <- max(subk$maxcutsz)
    totmx = max(mx,mxk)
    hasfailed = mx == 0
    hasfailed_k = mxk == 0

    if (i == 3 || i == 4) {
        hasfailed <- TRUE
        hasfailed_k <- TRUE
    }

    if (totmx < 500000) {
        hasfailed <- TRUE
        hasfailed_k <- TRUE
    }

    if (hasfailed_k && !hasfailed) {
        print("exception state quit: hasfailedk but not hasfailed")
        quit()
    }

    if (hasfailed) {
        data_table <- dplyr::filter(data_table, xsec != i)
        print(paste("Eliminated due to all maxcutsz 0: ", datasets[i+1], sep=""))
    } else {
        data_table$rmaxcutsz[data_table$xsec == i] <- data_table$maxcutsz[data_table$xsec == i] / totmx
    }

    if ((hasfailed && eliminate_in_kernelized_too) || hasfailed_k) {
        data_table_k <- dplyr::filter(data_table_k, xsec != i)
        print(paste("Eliminated due to all maxcutsz 0 (kernelized): ", datasets[i+1], sep=""))
    } else {
        data_table_k$rmaxcutsz[data_table_k$xsec == i] <- data_table_k$maxcutsz[data_table_k$xsec == i] / totmx
    }
}



{
    pdf(opt$out, width=10, height=5)
    par(cex = 1.2)

    y="rmaxcutsz"
    x="timex"

    # Define some ranges for our plotting area
    xrange <- range(data_table[,x])
    yrange <- range(0.8,1)
    xrange[2] <- 600

    plot(xrange, yrange, yaxt='n', xaxs='i', yaxs='i', col="black", type="n", main="stuff", ann=FALSE)
    axis(2, at=pretty(yrange), lab=paste0(pretty(yrange) * 100, '%'), las=TRUE)

    title(xlab="Time in seconds"     , line=2.3)
    title(ylab="Solution size (% of max found)", line=3.3)
    title(main=expression("Solution size over time by Localsolver: initial vs. kernelized graph"))

    c_legend_names <- c()
    for (i in 0:(length(datasets)-1)) {
        sub <- dplyr::filter(data_table, xsec == i)
        subk <- dplyr::filter(data_table_k, xsec == i)

        sub <- sub[with(sub, order(timex)), ]
        subk <- subk[with(subk, order(timex)), ]
        exists <- FALSE

        if (length(sub$timex) > 0) {
            if (!opt$nopoints) {
                points(sub[,x] , sub[,y] , col=col_vec[[i + 1]], pch=pnt_vec[[1]], cex=0.3)
            }

            if (printloess) {
                lo <- loess(sub[,y] ~ sub[,x], sub, span=loessv)
                lines(sub[,x], predict(lo), col=col_vec[[i + 1]], lwd=1.8, lty=2)
            }

            exists <- TRUE
        }

        if (length(subk$timex) > 0) {
            if (!opt$nopoints) {
                points(subk[,x] , subk[,y] , col=col_vec[[i + 1]], pch=pnt_vec[[1]], cex=0.3)
            }

            if (printloess) {
                lo <- loess(subk[,y] ~ subk[,x], subk, span=loessv)
                lines(subk[,x], predict(lo), col=col_vec[[i + 1]], lwd=1.8)
            }

            exists <- TRUE
        }

        if (exists) {
            c_legend_names <- c(c_legend_names, i + 1)
        }

       # points(subk[,x] , subk[,y] , col=col_vec[[i + 1]], pch=pnt_vec[[1]])
    }


    namevec <- datasets[c_legend_names]
    namevec <- lapply(namevec, function(x) basename(tools::file_path_sans_ext(x)))
  #  namevec <- lapply(namevec, function(x) gsub("(.*)/.*","\\0",x))


    legend("bottomright", yrange[2], namevec, lty=, col=col_vec[c_legend_names], pch=pnt_vec1)
}

#warnings()