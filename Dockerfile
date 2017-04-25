FROM ubuntu:16.04
RUN apt-get update && apt-get install -y build-essential

COPY . project/
WORKDIR project/
RUN make server && mkdir save/
EXPOSE 3000

ENTRYPOINT ["/project/server", "3000", "save/"]
