import http from "k6/http";

export default function() {
    const url = `${__ENV.SERVER}/`;
    http.get(url);
};
