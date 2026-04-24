(defproject wonk "0.1-SNAPSHOT"
  :license {:name "MIT License"
            :url "https://mit-license.org/"}
  :dependencies []
  :plugins [[org.jank-lang/lein-jank "0.6"]]
  :middleware [leiningen.jank/middleware]
  :main wonk.main
  :jank {:include-dirs ["include" 
                        "/usr/include/wlroots-0.20/"
                        ;; not sure about this one...
                        ;; not required for C ???
                        "/usr/include/pixman-1/"
                        ]
         :library-dirs ["/usr/lib/"]
         :linked-libraries ["wlroots-0.20",
                            "wayland-server"]}

  :profiles {:base {:jank {:output-dir "target/debug"
                           :optimization-level 0}}
             :release {:jank {:output-dir "target/release"
                              :optimization-level 2}}})
