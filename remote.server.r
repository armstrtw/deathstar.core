#!/usr/bin/env Rscript

library(rzmq)
library(getopt)

worker.id <- paste(Sys.info()["nodename"],Sys.getpid(),sep=":")

spec <- rbind(c("ready","r", 1, "character"),
              c("allocated","a", 1, "character"),
              c("work","w", 1, "character"),
              c("log-file","l", 1, "character"))

opts <- getopt(spec)
ready <- opts[["ready"]]
work <- opts[["work"]]
log.file <- opts[["log-file"]]

sink(log.file)
print(opts)

context = init.context()

ready.socket = init.socket(context,"ZMQ_PUSH")
work.socket = init.socket(context,"ZMQ_REP")

connect.socket(ready.socket,ready)
connect.socket(work.socket,work)



while(1) {
    ## send control message to indicate worker is up
    send.null.msg(ready.socket)

    ## wait for work
    msg = receive.socket(work.socket);

    index <- msg$index
    fun <- msg$fun
    args <- msg$args
    print(system.time(result <- try(do.call(fun,args),silent=TRUE)))
    send.socket(work.socket,list(index=index,result=result,node=worker.id));
    print(gc(verbose=TRUE))
}
