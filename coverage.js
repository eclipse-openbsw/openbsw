var parse = require('lcov-parse');
const fs = require('fs');


const lcovFilePath = './cmake-build-unit-tests/coverage.info'; 


const lcovString = fs.readFileSync(lcovFilePath, 'utf8');


parse(lcovString, function(err, data) {
    if (err) {
        console.error('Error parsing LCOV string:', err);
        return;
    }

    
    let totalLinesFound = 0;
    let totalLinesHit = 0;
    let totalFunctionsFound = 0;
    let totalFunctionsHit = 0;

    data.forEach(file => {
        if (file.lines) {
            totalLinesFound += file.lines.found || 0;
            totalLinesHit += file.lines.hit || 0;
        }
        if (file.functions) {
            totalFunctionsFound += file.functions.found || 0;
            totalFunctionsHit += file.functions.hit || 0;
        }
    });

    
    const totalLineCoverage = ((totalLinesHit / totalLinesFound) * 100).toFixed(2);
    const totalFunctionCoverage = ((totalFunctionsHit / totalFunctionsFound) * 100).toFixed(2);


    const formattedData = {
        total: {
            statements: {
                pct: totalLineCoverage
            },
            functions: {
                pct: totalFunctionCoverage
            }
        }
    };

    
    const outputFilePath = './coverage-badge/coverage-summary.json'; 
    fs.writeFileSync(outputFilePath, JSON.stringify(formattedData, null, 2));  


    console.log('Coverage data has been saved to:', outputFilePath);
    console.log(`Total Line Coverage: ${totalLineCoverage}%`);
    console.log(`Total Function Coverage: ${totalFunctionCoverage}%`);
});
