#! /usr/bin/env python

# Programs that are runnable.
ns3_runnable_programs = ['build/src/aodv/examples/ns3.29-aodv-debug', 'build/src/applications/examples/ns3.29-three-gpp-http-example-debug', 'build/src/bridge/examples/ns3.29-csma-bridge-debug', 'build/src/bridge/examples/ns3.29-csma-bridge-one-hop-debug', 'build/src/buildings/examples/ns3.29-buildings-pathloss-profiler-debug', 'build/src/config-store/examples/ns3.29-config-store-save-debug', 'build/src/core/examples/ns3.29-main-callback-debug', 'build/src/core/examples/ns3.29-sample-simulator-debug', 'build/src/core/examples/ns3.29-main-ptr-debug', 'build/src/core/examples/ns3.29-main-random-variable-stream-debug', 'build/src/core/examples/ns3.29-sample-random-variable-debug', 'build/src/core/examples/ns3.29-sample-random-variable-stream-debug', 'build/src/core/examples/ns3.29-command-line-example-debug', 'build/src/core/examples/ns3.29-hash-example-debug', 'build/src/core/examples/ns3.29-sample-log-time-format-debug', 'build/src/core/examples/ns3.29-test-string-value-formatting-debug', 'build/src/core/examples/ns3.29-main-test-sync-debug', 'build/src/csma/examples/ns3.29-csma-one-subnet-debug', 'build/src/csma/examples/ns3.29-csma-broadcast-debug', 'build/src/csma/examples/ns3.29-csma-packet-socket-debug', 'build/src/csma/examples/ns3.29-csma-multicast-debug', 'build/src/csma/examples/ns3.29-csma-raw-ip-socket-debug', 'build/src/csma/examples/ns3.29-csma-ping-debug', 'build/src/csma-layout/examples/ns3.29-csma-star-debug', 'build/src/dsdv/examples/ns3.29-dsdv-manet-debug', 'build/src/dsr/examples/ns3.29-dsr-debug', 'build/src/energy/examples/ns3.29-li-ion-energy-source-debug', 'build/src/energy/examples/ns3.29-rv-battery-model-test-debug', 'build/src/energy/examples/ns3.29-basic-energy-model-test-debug', 'build/src/fd-net-device/examples/ns3.29-dummy-network-debug', 'build/src/fd-net-device/examples/ns3.29-fd2fd-onoff-debug', 'build/src/fd-net-device/examples/ns3.29-realtime-dummy-network-debug', 'build/src/fd-net-device/examples/ns3.29-realtime-fd2fd-onoff-debug', 'build/src/fd-net-device/examples/ns3.29-fd-emu-ping-debug', 'build/src/fd-net-device/examples/ns3.29-fd-emu-udp-echo-debug', 'build/src/fd-net-device/examples/ns3.29-fd-emu-onoff-debug', 'build/src/fd-net-device/examples/ns3.29-fd-tap-ping-debug', 'build/src/fd-net-device/examples/ns3.29-fd-tap-ping6-debug', 'build/src/internet/examples/ns3.29-main-simple-debug', 'build/src/internet-apps/examples/ns3.29-dhcp-example-debug', 'build/src/lorawan/examples/ns3.29-simple-network-example-debug', 'build/src/lorawan/examples/ns3.29-network-server-example-debug', 'build/src/lorawan/examples/ns3.29-complete-network-example-debug', 'build/src/lorawan/examples/ns3.29-energy-model-example-debug', 'build/src/lr-wpan/examples/ns3.29-lr-wpan-packet-print-debug', 'build/src/lr-wpan/examples/ns3.29-lr-wpan-phy-test-debug', 'build/src/lr-wpan/examples/ns3.29-lr-wpan-data-debug', 'build/src/lr-wpan/examples/ns3.29-lr-wpan-error-model-plot-debug', 'build/src/lr-wpan/examples/ns3.29-lr-wpan-error-distance-plot-debug', 'build/src/lte/examples/ns3.29-lena-cqi-threshold-debug', 'build/src/lte/examples/ns3.29-lena-dual-stripe-debug', 'build/src/lte/examples/ns3.29-lena-fading-debug', 'build/src/lte/examples/ns3.29-lena-intercell-interference-debug', 'build/src/lte/examples/ns3.29-lena-ipv6-addr-conf-debug', 'build/src/lte/examples/ns3.29-lena-ipv6-ue-rh-debug', 'build/src/lte/examples/ns3.29-lena-ipv6-ue-ue-debug', 'build/src/lte/examples/ns3.29-lena-pathloss-traces-debug', 'build/src/lte/examples/ns3.29-lena-profiling-debug', 'build/src/lte/examples/ns3.29-lena-rem-debug', 'build/src/lte/examples/ns3.29-lena-rem-sector-antenna-debug', 'build/src/lte/examples/ns3.29-lena-rlc-traces-debug', 'build/src/lte/examples/ns3.29-lena-simple-debug', 'build/src/lte/examples/ns3.29-lena-simple-epc-debug', 'build/src/lte/examples/ns3.29-lena-deactivate-bearer-debug', 'build/src/lte/examples/ns3.29-lena-x2-handover-debug', 'build/src/lte/examples/ns3.29-lena-x2-handover-measures-debug', 'build/src/lte/examples/ns3.29-lena-frequency-reuse-debug', 'build/src/lte/examples/ns3.29-lena-distributed-ffr-debug', 'build/src/lte/examples/ns3.29-lena-uplink-power-control-debug', 'build/src/lte/examples/ns3.29-lena-simple-epc-emu-debug', 'build/src/mesh/examples/ns3.29-mesh-debug', 'build/src/mobility/examples/ns3.29-main-grid-topology-debug', 'build/src/mobility/examples/ns3.29-main-random-topology-debug', 'build/src/mobility/examples/ns3.29-main-random-walk-debug', 'build/src/mobility/examples/ns3.29-mobility-trace-example-debug', 'build/src/mobility/examples/ns3.29-ns2-mobility-trace-debug', 'build/src/mobility/examples/ns3.29-bonnmotion-ns2-example-debug', 'build/src/mpi/examples/ns3.29-simple-distributed-debug', 'build/src/mpi/examples/ns3.29-third-distributed-debug', 'build/src/mpi/examples/ns3.29-nms-p2p-nix-distributed-debug', 'build/src/mpi/examples/ns3.29-simple-distributed-empty-node-debug', 'build/src/netanim/examples/ns3.29-dumbbell-animation-debug', 'build/src/netanim/examples/ns3.29-grid-animation-debug', 'build/src/netanim/examples/ns3.29-star-animation-debug', 'build/src/netanim/examples/ns3.29-wireless-animation-debug', 'build/src/netanim/examples/ns3.29-uan-animation-debug', 'build/src/netanim/examples/ns3.29-colors-link-description-debug', 'build/src/netanim/examples/ns3.29-resources-counters-debug', 'build/src/network/examples/ns3.29-main-packet-header-debug', 'build/src/network/examples/ns3.29-main-packet-tag-debug', 'build/src/network/examples/ns3.29-packet-socket-apps-debug', 'build/src/nix-vector-routing/examples/ns3.29-nix-simple-debug', 'build/src/nix-vector-routing/examples/ns3.29-nms-p2p-nix-debug', 'build/src/olsr/examples/ns3.29-simple-point-to-point-olsr-debug', 'build/src/olsr/examples/ns3.29-olsr-hna-debug', 'build/src/point-to-point/examples/ns3.29-main-attribute-value-debug', 'build/src/propagation/examples/ns3.29-main-propagation-loss-debug', 'build/src/propagation/examples/ns3.29-jakes-propagation-model-example-debug', 'build/src/sixlowpan/examples/ns3.29-example-sixlowpan-debug', 'build/src/sixlowpan/examples/ns3.29-example-ping-lr-wpan-debug', 'build/src/spectrum/examples/ns3.29-adhoc-aloha-ideal-phy-debug', 'build/src/spectrum/examples/ns3.29-adhoc-aloha-ideal-phy-matrix-propagation-loss-model-debug', 'build/src/spectrum/examples/ns3.29-adhoc-aloha-ideal-phy-with-microwave-oven-debug', 'build/src/spectrum/examples/ns3.29-tv-trans-example-debug', 'build/src/spectrum/examples/ns3.29-tv-trans-regional-example-debug', 'build/src/stats/examples/ns3.29-gnuplot-example-debug', 'build/src/stats/examples/ns3.29-double-probe-example-debug', 'build/src/stats/examples/ns3.29-time-probe-example-debug', 'build/src/stats/examples/ns3.29-gnuplot-aggregator-example-debug', 'build/src/stats/examples/ns3.29-gnuplot-helper-example-debug', 'build/src/stats/examples/ns3.29-file-aggregator-example-debug', 'build/src/stats/examples/ns3.29-file-helper-example-debug', 'build/src/tap-bridge/examples/ns3.29-tap-csma-debug', 'build/src/tap-bridge/examples/ns3.29-tap-csma-virtual-machine-debug', 'build/src/tap-bridge/examples/ns3.29-tap-wifi-virtual-machine-debug', 'build/src/tap-bridge/examples/ns3.29-tap-wifi-dumbbell-debug', 'build/src/topology-read/examples/ns3.29-topology-example-sim-debug', 'build/src/traffic-control/examples/ns3.29-red-tests-debug', 'build/src/traffic-control/examples/ns3.29-red-vs-ared-debug', 'build/src/traffic-control/examples/ns3.29-adaptive-red-tests-debug', 'build/src/traffic-control/examples/ns3.29-pfifo-vs-red-debug', 'build/src/traffic-control/examples/ns3.29-codel-vs-pfifo-basic-test-debug', 'build/src/traffic-control/examples/ns3.29-codel-vs-pfifo-asymmetric-debug', 'build/src/traffic-control/examples/ns3.29-pie-example-debug', 'build/src/uan/examples/ns3.29-uan-cw-example-debug', 'build/src/uan/examples/ns3.29-uan-rc-example-debug', 'build/src/uan/examples/ns3.29-uan-raw-example-debug', 'build/src/uan/examples/ns3.29-uan-ipv4-example-debug', 'build/src/uan/examples/ns3.29-uan-ipv6-example-debug', 'build/src/uan/examples/ns3.29-uan-6lowpan-example-debug', 'build/src/virtual-net-device/examples/ns3.29-virtual-net-device-debug', 'build/src/wave/examples/ns3.29-wave-simple-80211p-debug', 'build/src/wave/examples/ns3.29-wave-simple-device-debug', 'build/src/wave/examples/ns3.29-vanet-routing-compare-debug', 'build/src/wifi/examples/ns3.29-wifi-phy-test-debug', 'build/src/wifi/examples/ns3.29-test-interference-helper-debug', 'build/src/wifi/examples/ns3.29-wifi-manager-example-debug', 'build/src/wifi/examples/ns3.29-wifi-trans-example-debug', 'build/src/wifi/examples/ns3.29-wifi-phy-configuration-debug', 'build/src/wimax/examples/ns3.29-wimax-ipv4-debug', 'build/src/wimax/examples/ns3.29-wimax-multicast-debug', 'build/src/wimax/examples/ns3.29-wimax-simple-debug', 'build/examples/realtime/ns3.29-realtime-udp-echo-debug', 'build/examples/naming/ns3.29-object-names-debug', 'build/examples/traffic-control/ns3.29-traffic-control-debug', 'build/examples/traffic-control/ns3.29-queue-discs-benchmark-debug', 'build/examples/traffic-control/ns3.29-red-vs-fengadaptive-debug', 'build/examples/traffic-control/ns3.29-red-vs-nlred-debug', 'build/examples/traffic-control/ns3.29-tbf-example-debug', 'build/examples/stats/ns3.29-wifi-example-sim-debug', 'build/examples/tutorial/ns3.29-hello-simulator-debug', 'build/examples/tutorial/ns3.29-first-debug', 'build/examples/tutorial/ns3.29-second-debug', 'build/examples/tutorial/ns3.29-third-debug', 'build/examples/tutorial/ns3.29-fourth-debug', 'build/examples/tutorial/ns3.29-fifth-debug', 'build/examples/tutorial/ns3.29-sixth-debug', 'build/examples/tutorial/ns3.29-seventh-debug', 'build/examples/udp/ns3.29-udp-echo-debug', 'build/examples/tcp/ns3.29-tcp-large-transfer-debug', 'build/examples/tcp/ns3.29-tcp-nsc-lfn-debug', 'build/examples/tcp/ns3.29-tcp-nsc-zoo-debug', 'build/examples/tcp/ns3.29-tcp-star-server-debug', 'build/examples/tcp/ns3.29-star-debug', 'build/examples/tcp/ns3.29-tcp-bulk-send-debug', 'build/examples/tcp/ns3.29-tcp-pcap-nanosec-example-debug', 'build/examples/tcp/ns3.29-tcp-nsc-comparison-debug', 'build/examples/tcp/ns3.29-tcp-variants-comparison-debug', 'build/examples/tcp/ns3.29-tcp-pacing-debug', 'build/examples/socket/ns3.29-socket-bound-static-routing-debug', 'build/examples/socket/ns3.29-socket-bound-tcp-static-routing-debug', 'build/examples/socket/ns3.29-socket-options-ipv4-debug', 'build/examples/socket/ns3.29-socket-options-ipv6-debug', 'build/examples/wireless/ns3.29-mixed-wired-wireless-debug', 'build/examples/wireless/ns3.29-wifi-adhoc-debug', 'build/examples/wireless/ns3.29-wifi-clear-channel-cmu-debug', 'build/examples/wireless/ns3.29-wifi-ap-debug', 'build/examples/wireless/ns3.29-wifi-wired-bridging-debug', 'build/examples/wireless/ns3.29-multirate-debug', 'build/examples/wireless/ns3.29-wifi-simple-adhoc-debug', 'build/examples/wireless/ns3.29-wifi-simple-adhoc-grid-debug', 'build/examples/wireless/ns3.29-wifi-simple-infra-debug', 'build/examples/wireless/ns3.29-wifi-simple-interference-debug', 'build/examples/wireless/ns3.29-wifi-blockack-debug', 'build/examples/wireless/ns3.29-dsss-validation-debug', 'build/examples/wireless/ns3.29-ofdm-validation-debug', 'build/examples/wireless/ns3.29-ofdm-ht-validation-debug', 'build/examples/wireless/ns3.29-ofdm-vht-validation-debug', 'build/examples/wireless/ns3.29-wifi-hidden-terminal-debug', 'build/examples/wireless/ns3.29-ht-wifi-network-debug', 'build/examples/wireless/ns3.29-vht-wifi-network-debug', 'build/examples/wireless/ns3.29-wifi-timing-attributes-debug', 'build/examples/wireless/ns3.29-wifi-sleep-debug', 'build/examples/wireless/ns3.29-power-adaptation-distance-debug', 'build/examples/wireless/ns3.29-power-adaptation-interference-debug', 'build/examples/wireless/ns3.29-rate-adaptation-distance-debug', 'build/examples/wireless/ns3.29-wifi-aggregation-debug', 'build/examples/wireless/ns3.29-simple-ht-hidden-stations-debug', 'build/examples/wireless/ns3.29-80211n-mimo-debug', 'build/examples/wireless/ns3.29-mixed-network-debug', 'build/examples/wireless/ns3.29-wifi-tcp-debug', 'build/examples/wireless/ns3.29-80211e-txop-debug', 'build/examples/wireless/ns3.29-wifi-spectrum-per-example-debug', 'build/examples/wireless/ns3.29-wifi-spectrum-per-interference-debug', 'build/examples/wireless/ns3.29-wifi-spectrum-saturation-example-debug', 'build/examples/wireless/ns3.29-ofdm-he-validation-debug', 'build/examples/wireless/ns3.29-he-wifi-network-debug', 'build/examples/wireless/ns3.29-wifi-multi-tos-debug', 'build/examples/wireless/ns3.29-wifi-backward-compatibility-debug', 'build/examples/wireless/ns3.29-wifi-pcf-debug', 'build/examples/matrix-topology/ns3.29-matrix-topology-debug', 'build/examples/udp-client-server/ns3.29-udp-client-server-debug', 'build/examples/udp-client-server/ns3.29-udp-trace-client-server-debug', 'build/examples/routing/ns3.29-dynamic-global-routing-debug', 'build/examples/routing/ns3.29-static-routing-slash32-debug', 'build/examples/routing/ns3.29-global-routing-slash32-debug', 'build/examples/routing/ns3.29-global-injection-slash32-debug', 'build/examples/routing/ns3.29-simple-global-routing-debug', 'build/examples/routing/ns3.29-simple-alternate-routing-debug', 'build/examples/routing/ns3.29-mixed-global-routing-debug', 'build/examples/routing/ns3.29-simple-routing-ping6-debug', 'build/examples/routing/ns3.29-manet-routing-compare-debug', 'build/examples/routing/ns3.29-ripng-simple-network-debug', 'build/examples/routing/ns3.29-rip-simple-network-debug', 'build/examples/routing/ns3.29-global-routing-multi-switch-plus-router-debug', 'build/examples/energy/ns3.29-energy-model-example-debug', 'build/examples/energy/ns3.29-energy-model-with-harvesting-example-debug', 'build/examples/error-model/ns3.29-simple-error-model-debug', 'build/examples/ipv6/ns3.29-icmpv6-redirect-debug', 'build/examples/ipv6/ns3.29-ping6-debug', 'build/examples/ipv6/ns3.29-radvd-debug', 'build/examples/ipv6/ns3.29-radvd-two-prefix-debug', 'build/examples/ipv6/ns3.29-test-ipv6-debug', 'build/examples/ipv6/ns3.29-fragmentation-ipv6-debug', 'build/examples/ipv6/ns3.29-fragmentation-ipv6-two-MTU-debug', 'build/examples/ipv6/ns3.29-loose-routing-ipv6-debug', 'build/examples/ipv6/ns3.29-wsn-ping6-debug', 'build/scratch/ns3.29-wifi-adhoc-var-file-size-4-flows-debug', 'build/scratch/ns3.29-wifi-adhoc-var-distance-3-flows-debug', 'build/scratch/ns3.29-wifi-adhoc-var-file-size-3-flows-debug', 'build/scratch/ns3.29-LoRa-var-dist-debug', 'build/scratch/ns3.29-wifi-adhoc-var-distance-debug', 'build/scratch/ns3.29-wifi-adhoc-var-file-size-2-flows-debug', 'build/scratch/ns3.29-wifi-adhoc-var-distance-random-debug', 'build/scratch/ns3.29-wifi-adhoc-var-distance-2-flows-debug', 'build/scratch/ns3.29-wifi-adhoc-var-file-size-random-debug', 'build/scratch/ns3.29-wifi-adhoc-var-distance-4-flows-debug', 'build/scratch/ns3.29-wifi-adhoc-var-file-size-debug', 'build/scratch/ns3.29-scratch-simulator-debug', 'build/scratch/subdir/ns3.29-subdir-debug']

# Scripts that are runnable.
ns3_runnable_scripts = ['csma-bridge.py', 'sample-simulator.py', 'wifi-olsr-flowmon.py', 'tap-csma-virtual-machine.py', 'tap-wifi-virtual-machine.py', 'realtime-udp-echo.py', 'first.py', 'second.py', 'third.py', 'mixed-wired-wireless.py', 'wifi-ap.py', 'simple-routing-ping6.py']

