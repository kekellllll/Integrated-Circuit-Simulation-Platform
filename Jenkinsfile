pipeline {
    agent any
    
    environment {
        BUILD_TYPE = 'Release'
        CUDA_VISIBLE_DEVICES = '0'
    }
    
    stages {
        stage('Checkout') {
            steps {
                echo 'Checking out source code...'
                checkout scm
            }
        }
        
        stage('Build Dependencies') {
            parallel {
                stage('C++/CUDA Build') {
                    steps {
                        echo 'Building C++/CUDA simulation engine...'
                        sh '''
                            mkdir -p build
                            cd build
                            cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ..
                            make -j$(nproc)
                        '''
                    }
                }
                
                stage('Web Dependencies') {
                    steps {
                        echo 'Installing web dependencies...'
                        dir('web') {
                            sh '''
                                npm install
                                npm run build
                            '''
                        }
                    }
                }
            }
        }
        
        stage('Unit Tests') {
            steps {
                echo 'Running unit tests...'
                sh '''
                    cd build
                    ctest --output-on-failure --parallel $(nproc)
                '''
            }
            post {
                always {
                    publishTestResults testResultsPattern: 'build/test-results/*.xml'
                }
            }
        }
        
        stage('Integration Tests') {
            steps {
                echo 'Running integration tests...'
                sh '''
                    cd build
                    ./bin/ic_simulator --test-mode
                    
                    # Test plugin loading
                    ./bin/ic_simulator --load-plugin ./lib/plugins/example_plugin.so
                    
                    # Test CUDA functionality if available
                    if command -v nvidia-smi >/dev/null 2>&1; then
                        echo "CUDA available, running GPU tests..."
                        ./bin/ic_simulator --test-cuda
                    else
                        echo "CUDA not available, skipping GPU tests"
                    fi
                '''
            }
        }
        
        stage('Regression Tests') {
            steps {
                echo 'Running regression tests...'
                script {
                    // Run simulation accuracy tests
                    sh '''
                        cd tests/regression
                        python3 run_regression_tests.py --tolerance 1e-6
                    '''
                    
                    // Performance regression tests
                    sh '''
                        cd tests/performance
                        python3 benchmark_simulation.py --baseline baseline_results.json
                    '''
                }
            }
            post {
                always {
                    archiveArtifacts artifacts: 'tests/regression/results/*.json', allowEmptyArchive: true
                    publishHTML([
                        allowMissing: false,
                        alwaysLinkToLastBuild: false,
                        keepAll: true,
                        reportDir: 'tests/regression/reports',
                        reportFiles: 'index.html',
                        reportName: 'Regression Test Report'
                    ])
                }
            }
        }
        
        stage('Web UI Tests') {
            steps {
                echo 'Running web interface tests...'
                dir('web') {
                    sh '''
                        # Start local server for testing
                        python3 -m http.server 8080 &
                        SERVER_PID=$!
                        
                        # Wait for server to start
                        sleep 2
                        
                        # Run web tests
                        npm run test
                        
                        # Stop server
                        kill $SERVER_PID
                    '''
                }
            }
        }
        
        stage('Code Quality') {
            parallel {
                stage('Static Analysis') {
                    steps {
                        echo 'Running static analysis...'
                        sh '''
                            # Run cppcheck for C++ code
                            cppcheck --enable=all --xml --xml-version=2 src/ include/ 2> cppcheck-result.xml || true
                            
                            # Run clang-tidy
                            find src include -name "*.cpp" -o -name "*.h" | xargs clang-tidy --checks='-*,readability-*,performance-*,modernize-*' || true
                        '''
                    }
                }
                
                stage('Code Coverage') {
                    steps {
                        echo 'Generating code coverage report...'
                        sh '''
                            cd build
                            make coverage || true
                            gcovr -r .. --xml-pretty --exclude-unreachable-branches --print-summary -o coverage.xml
                        '''
                    }
                }
                
                stage('Security Scan') {
                    steps {
                        echo 'Running security scans...'
                        sh '''
                            # Scan for common vulnerabilities
                            bandit -r src/ -f json -o bandit-report.json || true
                            
                            # Check for hardcoded secrets
                            truffleHog --regex --entropy=False . || true
                        '''
                    }
                }
            }
        }
        
        stage('Documentation') {
            steps {
                echo 'Generating documentation...'
                sh '''
                    cd build
                    make docs || true
                '''
            }
            post {
                always {
                    publishHTML([
                        allowMissing: true,
                        alwaysLinkToLastBuild: false,
                        keepAll: true,
                        reportDir: 'build/docs/html',
                        reportFiles: 'index.html',
                        reportName: 'API Documentation'
                    ])
                }
            }
        }
        
        stage('Package') {
            when {
                anyOf {
                    branch 'main'
                    branch 'release/*'
                }
            }
            steps {
                echo 'Creating packages...'
                sh '''
                    cd build
                    make package
                    
                    # Create Docker image
                    cd ..
                    docker build -t ic-simulator:latest .
                    docker save ic-simulator:latest > ic-simulator-docker.tar
                '''
            }
            post {
                always {
                    archiveArtifacts artifacts: 'build/*.tar.gz,build/*.deb,build/*.rpm,*.tar', allowEmptyArchive: true
                }
            }
        }
    }
    
    post {
        always {
            echo 'Cleaning up...'
            sh '''
                # Clean build artifacts but keep test results
                rm -rf build/CMakeFiles build/src build/examples
                
                # Archive important files
                tar -czf build-artifacts.tar.gz build/ web/dist/ || true
            '''
            
            // Publish test results
            publishTestResults testResultsPattern: 'build/test-results/*.xml'
            
            // Publish code coverage
            publishCoverageResults([
                adapters: [
                    gcovr(
                        path: 'build/coverage.xml',
                        mergeToOneReport: true
                    )
                ],
                sourceFileResolver: sourceFiles('STORE_ALL_BUILD')
            ])
            
            // Send notifications
            script {
                def status = currentBuild.currentResult
                def color = status == 'SUCCESS' ? 'good' : 'danger'
                def message = """
                    *${env.JOB_NAME}* - Build #${env.BUILD_NUMBER}
                    Status: *${status}*
                    Branch: ${env.BRANCH_NAME}
                    Duration: ${currentBuild.durationString}
                    
                    Changes:
                    ${getChangeString()}
                """
                
                slackSend(color: color, message: message)
            }
        }
        
        success {
            echo 'Pipeline completed successfully!'
        }
        
        failure {
            echo 'Pipeline failed!'
            // Send failure notifications
            emailext(
                subject: "Build Failed: ${env.JOB_NAME} - ${env.BUILD_NUMBER}",
                body: "The build has failed. Please check the Jenkins console output for details.",
                to: "${env.CHANGE_AUTHOR_EMAIL}"
            )
        }
    }
}

def getChangeString() {
    def changeString = ""
    def changes = currentBuild.changeSets
    for (int i = 0; i < changes.size(); i++) {
        def entries = changes[i].items
        for (int j = 0; j < entries.length; j++) {
            def entry = entries[j]
            changeString += "• ${entry.msg} (${entry.author})\\n"
        }
    }
    return changeString.isEmpty() ? "No changes" : changeString
}