# squim

[![Build Status](https://travis-ci.org/baranov1ch/squim.svg)](https://travis-ci.org/baranov1ch/squim)

grpc service for webp image compression (SQUashing IMages).

Sooo WIP.

But sooner or later it will be a gRPC-based service for compressing popular web images formats (jpeg, gif, png) to WebP.

As someone may know, Chromium-based browsers support WebP, and there are data compression proxies working in conjunction with some of them (Google Chrome Data Saver, Opera Turbo, Yandex Turbo) which, among other things, converts most images to webp, reducing  image size for 70% on average.

Soon, this project may give you a chance to do the same, at least for fun :)

Of course, there is great [mod_pagespeed](https://github.com/pagespeed/mod_pagespeed) project, which allows doing the same in apache/nginx module, but this is an effort to isolate image optimization into standalone service. In fact, many ideas, and even code pieces/test scenarios are taken from mod_pagespeed.
